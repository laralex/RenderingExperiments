#pragma once

#include "engine/Precompiled.hpp"

#include <optional>
#include <sys/inotify.h>


namespace engine::platform::linux {

class WatchedDirectory {

public:
    using FileChangedCallback = void (*)(std::string_view filepath, bool isDirectory);
#define Self WatchedDirectory
    explicit Self() noexcept = default;
    ~Self() noexcept;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&);
    Self& operator=(Self&&);
#undef Self
    static auto Allocate [[nodiscard]] (std::string_view watchDirectory, FileChangedCallback cb)
    -> std::optional<WatchedDirectory>;
    auto InotifyDescriptor [[nodiscard]] () const -> int { return inotifyFileDescriptor_; }
    auto WatchedDirectoryDescriptor [[nodiscard]] () const -> int { return watchDirectoryDescriptor_; }
    auto OnChangeCallback [[nodiscard]] () const -> FileChangedCallback { return fileChangedCallback_; }
    auto IsValid [[nodiscard]] () const -> bool {
        return inotifyFileDescriptor_ >= 0 && watchDirectoryDescriptor_ >= 0;
    }

private:
    int inotifyFileDescriptor_{-1};
    int watchDirectoryDescriptor_{-1};
    FileChangedCallback fileChangedCallback_{nullptr};
};

class FileChangeWatcher final {

public:
#define Self FileChangeWatcher
    explicit Self()              = default;
    ~Self() noexcept             = default;
    Self(Self const&)            = delete;
    Self& operator=(Self const&) = delete;
    Self(Self&&)                 = delete;
    Self& operator=(Self&&)      = delete;
#undef Self

    void PollEvents(WatchedDirectory const& directory) noexcept;

private:
    ENGINE_STATIC constexpr static size_t EVENT_SIZE        = sizeof(inotify_event);
    ENGINE_STATIC constexpr static size_t EVENT_BUFFER_SIZE = 1024U * EVENT_SIZE;
    char eventBuffer_[EVENT_BUFFER_SIZE]{};
};

} // namespace engine::platform::linux