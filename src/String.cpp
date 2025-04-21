#include "utils/String.h"

#ifdef _WIN32

namespace utils {

// Converts a narrow (UTF-8-encoded) string into a wide (UTF-16-encoded) string.
std::wstring
WidenString(std::string_view input) {
   if (input.empty()) {
      return {};
   }

   UINT  cp              = CP_UTF8;
   DWORD flags           = MB_ERR_INVALID_CHARS;
   auto  input_c         = input.data();
   auto  input_length    = static_cast<int>(input.size());
   auto  required_length = MultiByteToWideChar(cp, flags, input_c, input_length, nullptr, 0);
   if (required_length > 0) {
      std::wstring output(static_cast<std::size_t>(required_length), L'\0');
      if (MultiByteToWideChar(cp, flags, input_c, input_length, &output[0], required_length) > 0) {
         return output;
      }
   }
   // Failed to convert string from UTF-8 to UTF-16
   return {};
}

// Converts a wide (UTF-16-encoded) string into a narrow (UTF-8-encoded) string.
std::string
NarrowString(std::wstring_view input) {
   // WC_ERR_INVALID_CHARS
   static constexpr std::size_t ERR_INVALID_CHARS{0x00000080U};

   if (input.empty()) {
      return {};
   }
   UINT  cp           = CP_UTF8;
   DWORD flags        = ERR_INVALID_CHARS;
   auto  input_c      = input.data();
   auto  input_length = static_cast<int>(input.size());
   auto  required_length =
      WideCharToMultiByte(cp, flags, input_c, input_length, nullptr, 0, nullptr, nullptr);
   if (required_length > 0) {
      std::string output(static_cast<std::size_t>(required_length), '\0');
      if (WideCharToMultiByte(
             cp, flags, input_c, input_length, &output[0], required_length, nullptr, nullptr
          )
          > 0) {
         return output;
      }
   }
   // Failed to convert string from UTF-16 to UTF-8
   return {};
}

}  // namespace utils

#endif