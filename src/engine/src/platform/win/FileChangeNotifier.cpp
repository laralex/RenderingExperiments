#include "engine/platform/FileChangeNotifier.hpp"
#include "engine/platform/Filesystem.hpp"

namespace {

} // namespace anonymous

namespace engine::platform {

auto FileChangeNotifier::Initialize() noexcept -> bool {
    XLOGE("FileChangeNotifier::Initialize not implemented");
    return false;
}

auto FileChangeNotifier::IsInitialized() const noexcept -> bool {
    XLOGE("FileChangeNotifier::IsInitialized not implemented");
    return false;
}

auto FileChangeNotifier::PollChanges() noexcept -> bool {
    XLOGE("FileChangeNotifier::PollChanges not implemented");
    return false;
}

auto FileChangeNotifier::SubscribeWatcher(std::weak_ptr<IFileWatcher> watcher, std::string_view filepath) -> bool {
    XLOGE("FileChangeNotifier::SubscribeWatcher not implemented");
    return false;
}

void FileChangeNotifier::UnsubscribeWatcher(IFileWatcher const& deleteWatcher) {
    XLOGE("FileChangeNotifier::UnsubscribeWatcher not implemented");
}

auto FileChangeNotifier::UnsubscribeFile(std::string_view filepath) -> bool {
    XLOGE("FileChangeNotifier::UnsubscribeFile not implemented");
    return false;
}

void FileChangeNotifier::UnsubscribeAllWatchers() {
    XLOGE("FileChangeNotifier::UnsubscribeAllWatchers not implemented");
}

} // namespace engine::platform