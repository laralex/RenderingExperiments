#include "engine/platform/Filesystem.hpp"

namespace engine::platform {

auto AbsolutePath(std::string_view cwdRelativePath, std::error_code& err) -> std::filesystem::path {
    auto absolutePath = std::filesystem::path{
        std::begin(cwdRelativePath), std::end(cwdRelativePath), std::filesystem::path::generic_format};
    absolutePath = std::filesystem::weakly_canonical(absolutePath, err).string();
    return absolutePath;
}

} // namespace engine::platform