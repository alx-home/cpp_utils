#pragma once

#ifdef _WIN32
#   include <Windows.h>

#   include <string>
#   include <string_view>

namespace utils {

// Converts a narrow (UTF-8-encoded) string into a wide (UTF-16-encoded) string.
std::wstring WidenString(std::string_view input);

// Converts a wide (UTF-16-encoded) string into a narrow (UTF-8-encoded) string.
std::string NarrowString(std::wstring_view input);

}  // namespace utils
#endif

namespace utils {

template <size_t SIZE> struct String {
   constexpr explicit(false) String(const char (&str)[SIZE]);

   std::array<char, SIZE> value_{};
};

}  // namespace utils

#include "String.inl"