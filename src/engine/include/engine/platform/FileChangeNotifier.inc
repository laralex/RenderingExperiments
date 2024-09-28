explicit FileChangeNotifier()                            = default;
~FileChangeNotifier()                                    = default;
FileChangeNotifier(FileChangeNotifier const&)            = delete;
FileChangeNotifier& operator=(FileChangeNotifier const&) = delete;
FileChangeNotifier(FileChangeNotifier&&)                 = delete;
FileChangeNotifier& operator=(FileChangeNotifier&&)      = delete;

auto Initialize  [[nodiscard]] () noexcept -> bool;
auto IsInitialized [[nodiscard]] () const noexcept -> bool;
// returns false, when no more changes detected
auto PollChanges [[nodiscard]] () noexcept -> bool;
auto SubscribeWatcher [[nodiscard]](std::weak_ptr<IFileWatcher> watcher, std::string_view filepath) -> bool;
void UnsubscribeWatcher(IFileWatcher const& watcher);
auto UnsubscribeFile [[nodiscard]](std::string_view filepath) -> bool;
void UnsubscribeAllWatchers();