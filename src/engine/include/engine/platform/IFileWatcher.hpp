#pragma once

#include <string>

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

    virtual void OnFileChanged(std::string const& path, bool isDirectory) = 0;
};

} // namespace engien::platform