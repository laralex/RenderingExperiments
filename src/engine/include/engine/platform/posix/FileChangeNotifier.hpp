#pragma once

#include "engine/Precompiled.hpp"
#include "engine/platform/IFileWatcher.hpp"

#include <sys/inotify.h>
#include <filesystem>

namespace engine::platform {

class FileChangeNotifier final {

public:
    #include "engine/platform/FileChangeNotifier.inc"

private:
    void CloseWatchDescriptor(int watchDescriptor);

    struct WatchedFile {
        std::vector<std::weak_ptr<IFileWatcher>> watchers = {};
        std::filesystem::path filepath = {};
    };
    int inotifyDescriptor_ = -1;
    std::unordered_map<int, WatchedFile> descriptor2watchers_ = {};
    std::unordered_map<std::string, int> filename2descriptor_ = {};
    std::mutex fileWatchersMutex_ = {};

    constexpr static size_t EVENT_BUFFER_SIZE = 4096U;
    alignas(inotify_event) uint8_t eventBuffer_[EVENT_BUFFER_SIZE] = {};
};

} // namespace engine::platform