#include "utils/Nonce.h"

#include <chrono>
#include <format>
#include <string>

namespace utils {
std::string
Nonce(std::optional<std::size_t> seed) {
   if (seed) {
      return std::format("{:x}", std::hash<std::string>{}(std::to_string(*seed)));
   } else {
      return std::format(
        "{:x}",
        std::hash<std::string>{
        }(std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()))
      );
   }
}
}  // namespace utils