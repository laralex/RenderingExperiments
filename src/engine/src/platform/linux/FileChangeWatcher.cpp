#include "engine/platform/linux/FileChangeWatcher.hpp"
#include "engine/Precompiled.hpp"

#include "engine_private/Prelude.hpp"

#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <unistd.h>

namespace engine::platform::linux {

ENGINE_EXPORT auto WatchedDirectory::Allocate(std::string_view watchDirectory, FileChangedCallback cb)
    -> std::optional<WatchedDirectory> {
    WatchedDirectory out;
    int inotifyFileDescriptor = inotify_init1(IN_NONBLOCK);
    if (inotifyFileDescriptor < 0) {
        XLOGE("Failed to init FileChangeWatcher inotify_init for directory: {}", watchDirectory);
        return std::nullopt;
    }
    out.inotifyFileDescriptor_    = inotifyFileDescriptor;
    out.watchDirectoryDescriptor_ = inotify_add_watch(out.inotifyFileDescriptor_, watchDirectory.data(), IN_MODIFY);
    return std::optional{std::move(out)};
}

ENGINE_EXPORT WatchedDirectory::WatchedDirectory(WatchedDirectory&& other)
    : inotifyFileDescriptor_(other.inotifyFileDescriptor_)
    , watchDirectoryDescriptor_(other.watchDirectoryDescriptor_) {
    other.inotifyFileDescriptor_    = 0;
    other.watchDirectoryDescriptor_ = 0;
}

ENGINE_EXPORT WatchedDirectory& WatchedDirectory::operator=(WatchedDirectory&& other) {
    inotifyFileDescriptor_          = other.inotifyFileDescriptor_;
    watchDirectoryDescriptor_       = other.watchDirectoryDescriptor_;
    other.inotifyFileDescriptor_    = 0;
    other.watchDirectoryDescriptor_ = 0;
    return *this;
}

ENGINE_EXPORT WatchedDirectory::~WatchedDirectory() {
    if (inotifyFileDescriptor_ < 0) { return; }
    // set for non-blocking reads
    fcntl(inotifyFileDescriptor_, F_SETFL, fcntl(inotifyFileDescriptor_, F_GETFL) | O_NONBLOCK);
    if (watchDirectoryDescriptor_) {
        inotify_rm_watch(inotifyFileDescriptor_, watchDirectoryDescriptor_);
        watchDirectoryDescriptor_ = 0;
    }
    close(inotifyFileDescriptor_);
    inotifyFileDescriptor_ = 0;
}

ENGINE_EXPORT void FileChangeWatcher::PollEvents(WatchedDirectory const& directory) noexcept {
    // TODO: InotifyDescriptor should be 1 for the app
    // watch descriptors are 1 per directory and are stored in even structure "inotify_event"
    int bytesWritten = read(directory.InotifyDescriptor(), eventBuffer_, std::size(eventBuffer_));
    int i            = 0;
    while (i < bytesWritten) {
        inotify_event* event = reinterpret_cast<inotify_event*>(&eventBuffer_[i]);
        i += EVENT_SIZE + event->len;
        if (event->len == 0) { continue; }
        bool isDirectory = (event->mask & IN_ISDIR) != 0;
        if (event->wd != directory.WatchedDirectoryDescriptor()) { continue; }
        if (event->mask & IN_MODIFY) { directory.OnChangeCallback()({event->name, event->len}, isDirectory); }
    }
}
} // namespace engine::platform::linux