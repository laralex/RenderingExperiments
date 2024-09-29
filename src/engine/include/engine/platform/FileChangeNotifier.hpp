#pragma once

#if defined(XPLATFORM_WINDOWS)
#include "engine/platform/win/FileChangeNotifier.hpp"
#elif defined(XPLATFORM_LINUX)
#include "engine/platform/posix/FileChangeNotifier.hpp"
#else
#endif // XPLATFORM
