#pragma once

#include "engine/ShaderDefine.hpp"
#include "engine/platform/IFileWatcher.hpp"
#include <cstddef>
#include <memory>
#include <vector>
#include <unordered_set>

namespace engine::gl {

class GlContext;
class GpuProgram;

class GpuProgramRegistry final : public engine::platform::IFileWatcher {

public:
#define Self GpuProgramRegistry
    Self();
    ~Self() override             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self

    struct ProgramEntry {
        std::weak_ptr<GpuProgram> program           = {};
        constexpr static size_t MAX_NUM_SHADERS       = 4; // typically 2 for vertex-fragment pair, 1 for compute
        std::string shadersFilepaths[MAX_NUM_SHADERS] = {};
        std::vector<ShaderDefine> defines           = {};
    };

    void RegisterProgram (std::weak_ptr<GpuProgram> program,
        std::string_view vertexFilepath, std::string_view fragmentFilepath,
        std::vector<ShaderDefine>&& defines);
    void UnregisterProgram(GpuProgram const& program);

    void HotReloadPrograms(GlContext& gl);

    using FilepathsToWatch = std::unordered_set<std::string>;
    auto FilepathsToWatchBegin [[nodiscard]] () const -> FilepathsToWatch::const_iterator {
        return std::cbegin(filepathsToWatch_);
    }
    auto FilepathsToWatchEnd [[nodiscard]] () const -> FilepathsToWatch::const_iterator {
        return std::cend(filepathsToWatch_);
    }

private:
    void OnFileChanged(std::string const& path, bool isDirectory) override;

    std::vector<ProgramEntry> programs_ = {};
    std::vector<size_t> pendingHotReload_ = {};
    FilepathsToWatch filepathsToWatch_ = {};
};

} // namespace engine::gl