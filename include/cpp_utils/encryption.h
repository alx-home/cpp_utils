#pragma once
#include "utils/String.h"
#include <algorithm>
#include <string>
#include <string_view>

namespace cpp_utils {

/**
 * @brief Deobfuscates a string stored in the program's data section using a compile-time key.
 *
 * This is intended for use with strings that are obfuscated at compile-time (e.g., XOR, ROT, etc.).
 * The obfuscation method must match the one used at compile-time.
 *
 * @tparam Key The obfuscation key (e.g., single char for XOR, or string for more complex schemes).
 * @param obfuscated The obfuscated string data.
 * @return           The deobfuscated (plain) string.
 */
template <utils::String KEY>
std::string
DeobfuscateString(std::string_view obfuscated) {
   std::string result{obfuscated};
   for (std::size_t i = 0; i < result.size(); ++i) {
      result[i] = result[i] ^ KEY.value_[i % KEY.value_.size()];
   }
   return result;
}

/**
 * @brief Obfuscates a string at compile-time using a template key.
 *
 * This is intended for use with strings that are obfuscated at compile-time (e.g., XOR, ROT, etc.).
 * The obfuscation method must match the one used at runtime for deobfuscation.
 *
 * @tparam KEY The obfuscation key (e.g., single char for XOR, or string for more complex schemes).
 * @param plain The plain string data to obfuscate.
 * @return      The obfuscated string.
 */
template <utils::String KEY, utils::String VALUE>
std::string_view static constexpr ObfuscateString() {
   static constexpr auto                N = VALUE.value_.size();
   static constinit std::array<char, N> s__result{([]() constexpr {
      std::array<char, N> result{};
      for (std::size_t i = 0; i < N; ++i) {
         result[i] = VALUE.value_[i] ^ KEY.value_[i % KEY.value_.size()];
      }
      return result;
   })()};
   return std::string_view{s__result.data(), N};
}

/**
 * @brief Encrypts a string using Windows CryptoAPI with optional password entropy.
 *
 * Uses CryptProtectData to securely encrypt the input. The password is used as additional entropy.
 * Only the same Windows user (and password, if provided) can decrypt the data.
 *
 * @param input    The plaintext data to encrypt.
 * @param password Optional password/entropy to strengthen encryption (can be empty).
 * @return         Pair of (success, encrypted data as string).
 */
std::pair<bool, std::string> Encrypt(std::string_view input, std::string_view password);

/**
 * @brief Decrypts a string using Windows CryptoAPI with optional password entropy.
 *
 * Uses CryptUnprotectData to securely decrypt the input. The password must match the one used for
 * encryption. Only the same Windows user (and password, if provided) can decrypt the data.
 *
 * @param input    The encrypted data to decrypt.
 * @param password Optional password/entropy used during encryption (must match).
 * @return         Pair of (success, decrypted plaintext as string).
 */
std::pair<bool, std::string> Decrypt(std::string_view input, std::string_view password);

}  // namespace cpp_utils
