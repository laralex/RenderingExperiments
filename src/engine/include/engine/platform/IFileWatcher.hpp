#pragma once

#include <filesystem>

namespace engine::platform {

class IFileWatcher {
public:
#define Self IFileWatcher
    Self()                       = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
    virtual ~Self()              = default;
#undef Self

    virtual void OnFileChanged(std::filesystem::path const& watchedFilepath, std::string_view subpath, bool isDirectory) = 0;
};

} // namespace engien::platform