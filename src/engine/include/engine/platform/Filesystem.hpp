#pragma once

#include <filesystem>
#include <system_error>

namespace engine::platform {

auto AbsolutePath [[nodiscard]] (std::string_view cwdRelativePath, std::error_code& err) -> std::filesystem::path;

} // namespace engine::platform