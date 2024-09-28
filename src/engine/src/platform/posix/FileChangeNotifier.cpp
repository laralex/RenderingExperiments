#include "engine/platform/FileChangeNotifier.hpp"
#include "engine/platform/posix/FileChangeNotifier.hpp"

#include <memory>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <fcntl.h>

namespace engine::platform {

auto FileChangeNotifier::Initialize() noexcept -> bool {
    inotifyDescriptor_ = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (inotifyDescriptor_ >= 0) {
        return true;
    }
    XLOGE("Failed to init FileChangeNotifier. inotify_init");
    return false;
}

auto FileChangeNotifier::IsInitialized() const noexcept -> bool {
    return inotifyDescriptor_ >= 0;
}

auto FileChangeNotifier::PollChanges() noexcept -> bool {
    assert(IsInitialized());
    std::lock_guard const lock(fileWatchersMutex_);

    int bytesWritten = read(inotifyDescriptor_, eventBuffer_, std::size(eventBuffer_));
    int i            = 0;
    while (i < bytesWritten) {
        inotify_event* event = reinterpret_cast<inotify_event*>(&eventBuffer_[i]);
        i += sizeof(inotify_event) + event->len;
        //if (event->len == 0) { continue; }
        bool isDirectory = (event->mask & IN_ISDIR) != 0;
        if ((event->mask & IN_MODIFY) == 0) { continue; }
        auto const findWatchers = descriptor2watchers_.find(event->wd);
        for (auto& watcher : findWatchers->second.watchers) {
            std::shared_ptr w{watcher.lock()};
            if (!w) { continue; }
            w->OnFileChanged(findWatchers->second.filepath, std::string_view{event->name, event->len}, isDirectory);
        }
    }
    return bytesWritten > 0;
}

auto FileChangeNotifier::SubscribeWatcher(std::weak_ptr<IFileWatcher> watcher, std::string_view anyFilepath) -> bool {
    assert(IsInitialized());

    std::error_code err;
    auto filepath = std::filesystem::path{
        std::begin(anyFilepath), std::end(anyFilepath), std::filesystem::path::generic_format};
    filepath = std::filesystem::weakly_canonical(filepath, err);
    if (err) {
        return false;
    }

    std::lock_guard const lock(fileWatchersMutex_);
    auto find = filename2descriptor_.find(filepath);
    if (find == filename2descriptor_.cend()) {
        // start watching new file
        int watchDescriptor = inotify_add_watch(inotifyDescriptor_, filepath.c_str(), IN_MODIFY);
        filename2descriptor_.emplace(filepath, watchDescriptor);
        descriptor2watchers_.emplace(watchDescriptor, WatchedFile{
            .watchers = std::vector{std::move(watcher)},
            .filepath = std::move(filepath),
        });
    } else {
        // find first expired watcher
        auto& watchers = descriptor2watchers_[find->second].watchers;
        auto findExpired = std::find_if(std::begin(watchers), std::end(watchers), [](auto const& w) { return w.expired(); });
        if (findExpired != std::end(watchers)) {
            // replace
            findExpired->swap(watcher);
        } else {
            // add to the end
            watchers.push_back(std::move(watcher));
        }
    }
    return true;
}

void FileChangeNotifier::UnsubscribeWatcher(IFileWatcher const& deleteWatcher) {
    assert(IsInitialized());

    std::lock_guard const lock(fileWatchersMutex_);
    for (auto it = std::begin(descriptor2watchers_); it != std::end(descriptor2watchers_); ++it) {
        for (auto& watcher : it->second.watchers) {
            std::shared_ptr<IFileWatcher> w = watcher.lock();
            if (!w) { continue; }
            if (w.get() != &deleteWatcher) { continue; }
            watcher.reset();
            break;
        }
        if (it->second.watchers.size() == 0) {
            CloseWatchDescriptor(it->first);
            auto findFilename = std::find_if(std::begin(filename2descriptor_), std::end(filename2descriptor_),
                           [=](auto&& p) { return p.second == it->first; });
            descriptor2watchers_.erase(it);
            filename2descriptor_.erase(findFilename);
        }
    }
}

void FileChangeNotifier::CloseWatchDescriptor(int watchDescriptor) {
    assert(IsInitialized());
    fcntl(inotifyDescriptor_, F_SETFL, fcntl(inotifyDescriptor_, F_GETFL) | O_NONBLOCK);
    inotify_rm_watch(inotifyDescriptor_, watchDescriptor);
    close(watchDescriptor);
}

auto FileChangeNotifier::UnsubscribeFile(std::string_view filepath) -> bool {
    assert(IsInitialized());

    std::error_code err;
    auto relpath = std::filesystem::path{std::begin(filepath), std::end(filepath), std::filesystem::path::generic_format};
    auto fullpath = std::filesystem::weakly_canonical(relpath, err);
    if (err) {
        return false;
    }

    std::lock_guard const lock(fileWatchersMutex_);
    auto find = filename2descriptor_.find(fullpath);
    if (find != filename2descriptor_.cend()) {
        CloseWatchDescriptor(find->second);
        descriptor2watchers_.erase(find->second);
        filename2descriptor_.erase(find);
    }
    return true;
}

void FileChangeNotifier::UnsubscribeAllWatchers() {
    std::lock_guard const lock(fileWatchersMutex_);
    for (auto it = filename2descriptor_.begin(); it != filename2descriptor_.end(); ++it) {
        CloseWatchDescriptor(it->second);
        descriptor2watchers_.erase(it->second);
        filename2descriptor_.erase(it);
    }
    assert(filename2descriptor_.empty());
    descriptor2watchers_.clear();
}

} // namespace engine::platform