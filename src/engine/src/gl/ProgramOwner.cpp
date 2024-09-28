#include "engine/gl/ProgramOwner.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Shader.hpp"

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
    return *find->second.get();
}

auto GpuProgramOwner::ProvideProgram (GpuProgramHandle handle) const -> std::weak_ptr<GpuProgram> {
    auto find = programs_.find(handle);
    if (find == std::cend(programs_)) {
        return {};
    }
    return find->second;
}

auto GpuProgramOwner::DeleteProgram(GpuProgramHandle handle) -> bool {
        auto isDeleted = programs_.erase(handle) > 0;
        for (auto it = std::begin(hotLoadPrograms_); it != std::end(hotLoadPrograms_); ++it) {
            if (it->handle != handle) { continue; }
            hotLoadPrograms_.erase(it);
        }
        return isDeleted;
    }

auto GpuProgramOwner::LinkProgramFromFiles(GlContext const& gl,
    std::string_view vertexFilepath, std::string_view fragmentFilepath,
    std::vector<shader::Define>&& defines, std::string_view name, bool hotReload) -> std::optional<GpuProgramHandle> {

    std::error_code err;
    auto vertexFullFilepath = std::filesystem::path{
        std::begin(vertexFilepath), std::end(vertexFilepath), std::filesystem::path::generic_format};
    vertexFullFilepath = std::filesystem::weakly_canonical(vertexFullFilepath, err).string();
    assert(!err);
    auto fragmentFullFilepath = std::filesystem::path{
        std::begin(fragmentFilepath), std::end(fragmentFilepath), std::filesystem::path::generic_format};
    fragmentFullFilepath = std::filesystem::weakly_canonical(fragmentFullFilepath, err).string();
    assert(!err);

    auto maybeGpuProgram = ::engine::gl::LinkProgramFromFiles(gl,
        vertexFullFilepath.c_str(), fragmentFullFilepath.c_str(),
        CpuView{defines.data(), defines.size()}, name, false);
    if (!maybeGpuProgram) { return std::nullopt; }
    GpuProgram gpuProgram = std::move(*maybeGpuProgram);
    GpuProgramHandle freeHandle = std::size(programs_);
    programs_[freeHandle] = std::make_shared<GpuProgram>(std::move(gpuProgram));

    if (hotReload) {
        hotLoadPrograms_.push_back(HotLoadProgram{
            .handle = freeHandle,
            .shadersFilepaths = {vertexFullFilepath, fragmentFullFilepath, {}, {}},
            .defines = std::move(defines)
        });
    }
    return std::optional{freeHandle};
}

void GpuProgramOwner::OnFileChanged(std::string const& path, bool isDirectory) {
    XLOG("ProgramOwner detected change {} {}", path.c_str(), isDirectory);
    // TODO: string copying (though hot reloading is rare, so maybe fine)
    bool isVert = HasEnding(path, shader::VERTEX_FILE_EXTENSION);
    bool isFrag = HasEnding(path, shader::FRAGMENT_FILE_EXTENSION);
    bool isCompute = HasEnding(path, shader::COMPUTE_FILE_EXTENSION);

    for (auto it = hotLoadPrograms_.cbegin(); it != hotLoadPrograms_.cend(); ++it) {
        bool pathEq[HotLoadProgram::MAX_NUM_SHADERS] = {
            std::filesystem::equivalent(it->shadersFilepaths[0], path),
            std::filesystem::equivalent(it->shadersFilepaths[1], path),
            std::filesystem::equivalent(it->shadersFilepaths[2], path),
            std::filesystem::equivalent(it->shadersFilepaths[3], path),
        };
        XLOG("v {} {} f {} {} c {} {}",
            isVert, pathEq[0],
            isFrag, pathEq[1],
            isCompute, pathEq[0]);
        bool doRecompile = (isVert && pathEq[0] || isFrag && pathEq[1] || isCompute && pathEq[0]);
        if (!doRecompile) { continue; }

        bool ok = false;
        GlContext gl; // TODO: works, but should be provided externally, for that maybe should poll pending changes
        if (isVert || isFrag) {
            ok = RelinkProgramFromFiles(gl,
                it->shadersFilepaths[0].c_str(), it->shadersFilepaths[1].c_str(),
                CpuView{it->defines.data(), it->defines.size()},
                *programs_[it->handle].get(), false);
        }
        if (!ok) {
            XLOGE("ProgramOwner failed to relink program for hot-reloaded shader {}", path.c_str());
        }
    }
}

} // namespace engine::gl