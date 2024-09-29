#pragma once

#include "engine/Precompiled.hpp"
#include "engine/platform/IFileWatcher.hpp"

namespace engine::platform {

class FileChangeNotifier final {

public:
#include "engine/platform/FileChangeNotifier.inc"

private:
    // TODO: implement for Windows
};

} // namespace engine::platform