#include "engine/gl/ProgramOwner.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/platform/Filesystem.hpp"

namespace {

auto HasEnding [[nodiscard]](std::string_view const str, std::string_view ending, size_t endOffset = 0) -> bool {
    if (str.length() < ending.length() + endOffset) {
        return false;
    }
    return str.compare(str.length() - ending.length() - endOffset, ending.length(), ending) == 0;
}

} // namespace anonymous

namespace engine::gl {

auto GpuProgramOwner::ViewProgram(GpuProgramHandle handle) const -> GpuProgram const& {
    auto find = programs_.find(handle);
    assert(find != std::cend(programs_));
    return *find->second.program.get();
}

auto GpuProgramOwner::ProvideProgram (GpuProgramHandle handle) const -> std::weak_ptr<GpuProgram> {
    auto find = programs_.find(handle);
    if (find == std::cend(programs_)) {
        return {};
    }
    return find->second.program;
}

auto GpuProgramOwner::DeleteProgram(GpuProgramHandle handle) -> bool {
        auto isDeleted = programs_.erase(handle) > 0;
        return isDeleted;
    }

auto GpuProgramOwner::LinkProgramFromFiles(GlContext const& gl,
    std::string_view vertexFilepath, std::string_view fragmentFilepath,
    std::vector<shader::Define>&& defines, std::string_view name, bool hotReload, bool logCode) -> std::optional<GpuProgramHandle> {

    std::error_code err;
    std::string vertexFullFilepath, fragmentFullFilepath;
    vertexFullFilepath = platform::AbsolutePath(vertexFilepath, err).string();
    assert(!err);
    fragmentFullFilepath = platform::AbsolutePath(fragmentFilepath, err).string();
    assert(!err);

    auto maybeGpuProgram = ::engine::gl::LinkProgramFromFiles(gl,
        vertexFullFilepath.c_str(), fragmentFullFilepath.c_str(),
        CpuView{defines.data(), defines.size()}, name, logCode);
    if (!maybeGpuProgram) { return std::nullopt; }

    GpuProgram gpuProgram = std::move(*maybeGpuProgram);
    GpuProgramHandle freeHandle = std::size(programs_);
    programs_[freeHandle] = HotLoadProgram{
        .program = std::make_shared<GpuProgram>(std::move(gpuProgram)),
        .hotReload = hotReload,
        .shadersFilepaths = {vertexFullFilepath, fragmentFullFilepath, {}, {}},
        .defines = std::move(defines)
    };
    if (hotReload) {
        filepathsToWatch_.insert(vertexFullFilepath);
        filepathsToWatch_.insert(fragmentFullFilepath);
    }
    return std::optional{freeHandle};
}

void GpuProgramOwner::OnFileChanged(std::string const& path, bool isDirectory) {
    XLOG("GpuProgramOwner detected change {}", path.c_str());
    bool isVert = HasEnding(path, shader::VERTEX_FILE_EXTENSION);
    bool isFrag = HasEnding(path, shader::FRAGMENT_FILE_EXTENSION);
    bool isCompute = HasEnding(path, shader::COMPUTE_FILE_EXTENSION);

    for (auto it = programs_.cbegin(); it != programs_.cend(); ++it) {
        if (!it->second.hotReload) { continue; }

        bool pathEq[HotLoadProgram::MAX_NUM_SHADERS];
        for (size_t i = 0; i < std::size(pathEq); ++i) {
            pathEq[i] = std::filesystem::equivalent(it->second.shadersFilepaths[i], path);
        };

        bool doRecompile = (isVert && pathEq[0] || isFrag && pathEq[1] || isCompute && pathEq[0]);
        if (!doRecompile) { continue; }

        pendingHotReload_.push_back(it->first);

    }
}

void GpuProgramOwner::HotReloadPrograms(GlContext const& gl) {
    for(auto handle : pendingHotReload_) {
        bool ok = false;
        auto find = programs_.find(handle);
        if (find == std::cend(programs_)) { continue; }
        auto const& payload = find->second;
        auto programType = payload.program->Type();
        if (programType == GpuProgram::Type::GRAPHICAL) {
            ok = RelinkProgramFromFiles(gl,
                payload.shadersFilepaths[0].c_str(), payload.shadersFilepaths[1].c_str(),
                CpuView{payload.defines.data(), payload.defines.size()},
                *payload.program.get(), false);
        } else if (programType == GpuProgram::Type::COMPUTE) {
            assert(false && "GpuProgramOwner::OnFileChanged not implemented for compute shaders");
        }
        if (!ok) {
            XLOGE("ProgramOwner failed to hot-reload program: {:08X}", payload.program->Id());
        }
    }
    pendingHotReload_.clear();
}

} // namespace engine::gl