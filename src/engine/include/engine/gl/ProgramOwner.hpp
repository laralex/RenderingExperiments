#pragma once

#include "engine/gl/Shader.hpp"
#include "engine/platform/FileChangeNotifier.hpp"
#include <cstddef>
#include <memory>
#include <optional>

namespace engine::gl {

class GlContext;
class GpuProgram;

class GpuProgramOwner final : public engine::platform::IFileWatcher {

public:
#define Self GpuProgramOwner
    Self()                       = default;
    ~Self()                      = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

    struct HotLoadProgram {
        std::shared_ptr<GpuProgram> program = {};
        bool hotReload = false;
        constexpr static size_t MAX_NUM_SHADERS = 4; // typically 2 for vertex-fragment pair, 1 for compute
        std::string shadersFilepaths[MAX_NUM_SHADERS] = {};
        std::vector<shader::Define> defines = {};
    };

    auto ViewProgram [[nodiscard]](GpuProgramHandle handle) const -> GpuProgram const&;

    auto ProvideProgram [[nodiscard]](GpuProgramHandle handle) const -> std::weak_ptr<GpuProgram>;

    auto DeleteProgram [[nodiscard]](GpuProgramHandle handle) -> bool;

    using FilepathsToWatch = std::unordered_set<std::string>;
    auto FilepathsToWatchBegin [[nodiscard]]() const -> FilepathsToWatch::const_iterator { return std::cbegin(filepathsToWatch_); }
    auto FilepathsToWatchEnd [[nodiscard]]() const -> FilepathsToWatch::const_iterator { return std::cend(filepathsToWatch_); }

    auto LinkProgramFromFiles[[nodiscard]](GlContext const& gl,
        std::string_view vertexFilepath, std::string_view fragmentFilepath,
        std::vector<shader::Define>&& defines, std::string_view name,
        bool hotReload = true, bool logCode = false) -> std::optional<GpuProgramHandle>;

    void HotReloadPrograms(GlContext const& gl);

private:

    void OnFileChanged(std::string const& path, bool isDirectory) override;

    std::unordered_map<GpuProgramHandle, HotLoadProgram> programs_ = {};
    std::vector<GpuProgramHandle> pendingHotReload_ = {};
    FilepathsToWatch filepathsToWatch_ = {};
};

} // engine::gl