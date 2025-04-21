#pragma once

#include "String.h"

#include <array>

namespace utils {

template <size_t SIZE> constexpr String<SIZE>::String(const char (&str)[SIZE]) {
   std::copy_n(str, SIZE, value_.begin());
}

}  // namespace utils