#include "engine/gl/GpuProgramRegistry.hpp"
#include "engine/gl/GpuProgram.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/platform/Filesystem.hpp"
#include <utility>

namespace {

auto HasEnding [[nodiscard]] (std::string_view const str, std::string_view ending, size_t endOffset = 0) -> bool {
    if (str.length() < ending.length() + endOffset) { return false; }
    return str.compare(str.length() - ending.length() - endOffset, ending.length(), ending) == 0;
}

constexpr size_t DEFAULT_CAPACITY = 64;

} // namespace

namespace engine::gl {

GpuProgramRegistry::GpuProgramRegistry() {
    programs_.reserve(DEFAULT_CAPACITY);
    pendingHotReload_.reserve(DEFAULT_CAPACITY);
    filepathsToWatch_.reserve(DEFAULT_CAPACITY);
}

void GpuProgramRegistry::RegisterProgram(
    std::weak_ptr<GpuProgram> program, std::string_view vertexFilepath, std::string_view fragmentFilepath,
    std::vector<ShaderDefine>&& defines) {
    //assert(false && "Rename to GpuProgramRegistry move hot-reloading to new GpuProgramHotreloader");
    std::error_code err;
    std::string vertexFullFilepath, fragmentFullFilepath;
    vertexFullFilepath = platform::AbsolutePath(vertexFilepath, err).string();
    assert(!err);
    fragmentFullFilepath = platform::AbsolutePath(fragmentFilepath, err).string();
    assert(!err);

    // TODO: issue, two copies of paths stored, once in programs_, once in filepathsToWatch_
    filepathsToWatch_.insert(vertexFullFilepath);
    filepathsToWatch_.insert(fragmentFullFilepath);

    // try to replace expired weak_ptr
    bool inserted = false;
    if (std::size(programs_) > 0) {
        for (auto& p : programs_) {
            if (!p.program.expired()) { continue; }
            p.program = program;
            p.shadersFilepaths[0] = std::move(vertexFullFilepath);
            p.shadersFilepaths[1] = std::move(fragmentFullFilepath);
            p.shadersFilepaths[2] = {};
            p.shadersFilepaths[3] = {};
            p.defines = std::move(defines);
            inserted = true;
            break;
        }
    }

    // everything is alive, add a new entry
    if (!inserted) {
        programs_.push_back(ProgramEntry {
            .program = program,
            .shadersFilepaths = {std::move(vertexFullFilepath), std::move(fragmentFullFilepath), {}, {}}, // TODO
            .defines = std::move(defines),
        });
    }
}

void GpuProgramRegistry::UnregisterProgram(GpuProgram const& program) {
    for (auto& p : programs_) {
        if (p.program.lock().get() != &program) { continue; }
        p.program.reset();
    }
}

void GpuProgramRegistry::OnFileChanged(std::string const& path, bool isDirectory) {
    XLOG("GpuProgramRegistry detected change {}", path.c_str());
    bool isVert    = HasEnding(path, shader::VERTEX_FILE_EXTENSION);
    bool isFrag    = HasEnding(path, shader::FRAGMENT_FILE_EXTENSION);
    bool isCompute = HasEnding(path, shader::COMPUTE_FILE_EXTENSION);

    for (size_t i = 0; i < std::size(programs_); ++i) {
        auto& entry = programs_[i];
        bool doRecompile = (isVert && std::filesystem::equivalent(entry.shadersFilepaths[0], path) || isFrag && std::filesystem::equivalent(entry.shadersFilepaths[1], path) || isCompute && std::filesystem::equivalent(entry.shadersFilepaths[0], path));
        if (!doRecompile) { continue; }

        pendingHotReload_.push_back(i);
    }
}

void GpuProgramRegistry::HotReloadPrograms(GlContext const& gl) {
    for (size_t idx : pendingHotReload_) {
        bool ok   = false;
        auto const& payload = programs_[idx];
        if (payload.program.expired()) { continue; }

        auto program = payload.program.lock();
        auto programType = program->Type();
        if (programType == GpuProgramType::GRAPHICAL) {
            ok = RelinkProgramFromFiles(
                gl, payload.shadersFilepaths[0], payload.shadersFilepaths[1],
                CpuView{payload.defines.data(), payload.defines.size()}, *program, false);
        } else if (programType == GpuProgramType::COMPUTE) {
            assert(false && "GpuProgramRegistry::OnFileChanged not implemented for compute shaders");
        }
        if (!ok) { XLOGE("GpuProgramRegistry failed to hot-reload program: {:08X}", program->Id()); }
    }
    pendingHotReload_.clear();
}

} // namespace engine::gl
