#pragma once

#include "engine/gl/Common.hpp"
#include "engine/gl/Context.hpp"
#include "engine/gl/Shader.hpp"
#include "engine/platform/FileChangeNotifier.hpp"
#include "engine/gl/Program.hpp"
#include "engine/gl/Shader.hpp"
#include <cstddef>
#include <memory>
#include <optional>

namespace engine::gl {

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

    using Handle = uint32_t;

    auto ViewProgram [[nodiscard]](Handle handle) -> GpuProgram const& {
        auto find = programs_.find(handle);
        assert(find != std::cend(programs_));
        return *find->second.get();
    }

    auto ProvideProgram [[nodiscard]](Handle handle) -> std::weak_ptr<GpuProgram> {
        auto find = programs_.find(handle);
        if (find == std::cend(programs_)) {
            return {};
        }
        return find->second;
    }

    auto DeleteProgram [[nodiscard]](Handle handle) -> bool {
        auto isDeleted = programs_.erase(handle) > 0;
        for (auto it = std::begin(hotLoadPrograms_); it != std::end(hotLoadPrograms_); ++it) {
            if (it->handle != handle) { continue; }
            hotLoadPrograms_.erase(it);
        }
        return isDeleted;
    }

    auto LinkProgramFromFiles[[nodiscard]](GlContext const& gl,
        std::string_view vertexFilepath, std::string_view fragmentFilepath,
        std::vector<shader::Define>&& defines, std::string_view name) -> std::optional<Handle> {
        
        std::error_code err;
        auto vertexFullFilepath = std::filesystem::path{
            std::begin(vertexFilepath), std::end(vertexFilepath), std::filesystem::path::generic_format};
        vertexFullFilepath = std::filesystem::weakly_canonical(vertexFullFilepath, err);
        assert(!err);
        auto fragmentFullFilepath = std::filesystem::path{
            std::begin(fragmentFilepath), std::end(fragmentFilepath), std::filesystem::path::generic_format};
        fragmentFullFilepath = std::filesystem::weakly_canonical(fragmentFullFilepath, err);
        assert(!err);

        auto maybeGpuProgram = ::engine::gl::LinkProgramFromFiles(gl,
            vertexFullFilepath.c_str(), fragmentFullFilepath.c_str(),
            CpuView{defines.data(), defines.size()}, name, false);
        if (!maybeGpuProgram) { return std::nullopt; }
        GpuProgram gpuProgram = std::move(*maybeGpuProgram);
        Handle freeHandle = std::size(programs_);
        programs_[freeHandle] = std::make_shared<GpuProgram>(std::move(gpuProgram));

        hotLoadPrograms_.push_back(HotLoadProgram{
            .handle = freeHandle,
            .shadersFilepaths = {vertexFullFilepath, fragmentFullFilepath, {}, {}},
            .defines = std::move(defines)
        });
        return std::optional{freeHandle};
    }

private:

    struct HotLoadProgram {
        Handle handle;
        constexpr static size_t MAX_NUM_SHADERS = 4; // typically 2 for vertex-fragment pair, 1 for compute
        std::filesystem::path shadersFilepaths[MAX_NUM_SHADERS];
        std::vector<shader::Define> defines;
    };

    void OnFileChanged(std::filesystem::path const& watchedFilepath, std::string_view subpath, bool isDirectory) override {
        XLOG("ProgramOwner detected change {} {} {}", watchedFilepath.c_str(), subpath, isDirectory);
        std::string fullpath = watchedFilepath.c_str();
        bool isVert = HasEnding(fullpath.substr(), shader::VERTEX_FILE_EXTENSION, 1);
        bool isFrag = HasEnding(fullpath.substr(), shader::FRAGMENT_FILE_EXTENSION, 1);
        isFrag = true;
        bool isCompute = HasEnding(fullpath.substr(), shader::COMPUTE_FILE_EXTENSION, 1);
        for (auto it = hotLoadPrograms_.cbegin(); it != hotLoadPrograms_.cend(); ++it) {
            bool doRecompile = (isVert && it->shadersFilepaths[0] == fullpath)
                || (isFrag && it->shadersFilepaths[1] == fullpath)
                || (isCompute && it->shadersFilepaths[0] == fullpath);
            if (!doRecompile) { continue; }
            GlContext gl; // TODO: works, but should be provided externally, for that maybe should poll pending changes
            bool ok = RelinkProgramFromFiles(gl,
                it->shadersFilepaths[0].c_str(), it->shadersFilepaths[1].c_str(),
                CpuView{it->defines.data(), it->defines.size()},
                *programs_[it->handle].get(), false);
            if (!ok) {
                XLOGE("ProgramOwner failed to relink program for hot-reloaded shader {} {}", watchedFilepath.c_str(), subpath);
            }
        }
    }

    auto HasEnding [[nodiscard]](std::string_view const str, std::string_view ending, size_t endOffset = 0) -> bool {
        if (str.length() < ending.length() + endOffset) {
            return false;
        }
        return str.compare(str.length() - ending.length() - endOffset, ending.length(), ending) == 0;
    }

    std::vector<HotLoadProgram> hotLoadPrograms_ = {};
    std::unordered_map<Handle, std::shared_ptr<GpuProgram>> programs_ = {};
};

} // engine::gl