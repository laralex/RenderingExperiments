#pragma once

#include <string>

namespace engine::platform {

class IFileWatcher {
public:
#define Self IFileWatcher
    Self()                       = default;
    virtual ~Self()              = default;
    virtual void OnFileChanged(std::string const& path, bool isDirectory) = 0;

protected:
    Self(Self const&)            = default;
    Self& operator=(Self const&) = default;
    Self(Self&&)                 = default;
    Self& operator=(Self&&)      = default;
#undef Self
};

} // namespace engine::platform