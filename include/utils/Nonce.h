#pragma once

#include <optional>
#include <string>

namespace utils {
std::string Nonce(std::optional<std::size_t> seed = std::nullopt);
}