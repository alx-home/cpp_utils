/*
MIT License

Copyright (c) 2025 Alexandre GARCIN

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include "utils/String.h"

#include "uuid.h"

#include <iostream>
#include <source_location>
#include <string>
#include <string_view>
#include <vector>
#include <cstddef>

namespace cpp_utils {

/**
 * @brief Concatenates two std::array objects into a single std::array at compile time.
 *
 * This consteval function takes two std::array objects of the same type but possibly different
 * sizes, and returns a new std::array containing all elements of the first array followed by all
 * elements of the second array.
 *
 * @tparam T   The type of the elements in the arrays.
 * @tparam N1  The size of the first array.
 * @tparam N2  The size of the second array.
 * @param a    The first input array.
 * @param b    The second input array.
 * @return std::array<T, N1 + N2>  A new array containing all elements of 'a' followed by all
 * elements of 'b'.
 */
template <typename T, std::size_t N1, std::size_t N2>
consteval std::array<T, N1 + N2>
Concat(std::array<T, N1> const& a, std::array<T, N2> const& b) {
   std::array<T, N1 + N2> out{};

   for (std::size_t i = 0; i < N1; ++i) {
      out[i] = a[i];
   }

   for (std::size_t i = 0; i < N2; ++i) {
      out[N1 + i] = b[i];
   }

   return out;
}

/**
 * @brief Computes a hash value using the FNV-1a algorithm with additional mixing.
 *
 * This function applies the FNV-1a hash algorithm, incorporating a single byte value (`v`)
 * into an existing hash value (`h`). It uses FNV and Murmur-inspired mixing steps to achieve
 * good distribution and avalanche properties.
 *
 * @param h The initial hash value to be updated.
 * @param v The byte value to mix into the hash.
 * @return The updated hash value after mixing in the byte.
 */
consteval std::size_t
Hash(std::size_t h, std::uint8_t v) {
   h ^= v;
   h *= 0x100000001b3ULL;
   h ^= (h >> 32);
   h *= 0xff51afd7ed558ccdULL;
   h ^= (h >> 33);

   return h;
}

/**
 * @brief Generates a unique compile-time identifier based on the current time and invocation count.
 *
 * This consteval function generates a unique identifier by hashing the current compilation time
 * (using the __TIME__ macro) combined with an incrementing ID. The resulting identifier is an
 * array of bytes that can be used for various purposes, such as unique keys or identifiers in
 * compile-time contexts.
 *
 * @tparam SIZE The size of the resulting identifier in bytes (default is 1, resulting in 8 bytes).
 * @tparam ID   The starting ID for uniqueness (default is 0).
 * @tparam FUN  A dummy template parameter to force re-instantiation (default is a lambda).
 * @return std::array<char, 8 * SIZE> A unique identifier as an array of bytes.
 */
struct UniqueIdTag;
template <std::size_t SIZE = 1>
consteval auto
UniqueId(std::source_location const& source = std::source_location::current()) {
   std::array<char, sizeof(PROJECT_BUILD_UUID)> time{PROJECT_BUILD_UUID};
   std::array<char, sizeof(std::size_t) * SIZE> result{};

   auto const filename = source.file_name();

   std::size_t hash = 0xcbf29ce484222325ULL;

   hash = Hash(hash, time[0]);
   hash = Hash(hash, time[1]);
   hash = Hash(hash, time[3]);
   hash = Hash(hash, time[4]);
   hash = Hash(hash, time[6]);
   hash = Hash(hash, time[7]);

   for (std::size_t i = 0; i < sizeof(source.line()); ++i) {
      hash = Hash(hash, static_cast<std::uint8_t>((source.line() >> (i * 8)) & 0xFF));
   }
   for (std::size_t i = 0; i < sizeof(source.column()); ++i) {
      hash = Hash(hash, static_cast<std::uint8_t>((source.column() >> (i * 8)) & 0xFF));
   }

   for (std::size_t j = 0; j < SIZE; ++j) {
      for (std::size_t i = 0; filename[i] != '\0'; ++i) {
         hash = Hash(hash, static_cast<std::uint8_t>(filename[i]));
      }

      for (std::size_t i = 0; i < sizeof(std::size_t); ++i) {
         result[j * sizeof(std::size_t) + i] = static_cast<char>((hash >> (i * 8)) & 0xFF);
      }
   }

   return result;
}

static_assert(sizeof(UniqueId()) == 8, "UniqueId should be 8 bytes long");
static_assert(sizeof(UniqueId<2>()) == 16, "UniqueId should be 16 bytes long");
static_assert(
  []() consteval {
     auto id1 = UniqueId();
     auto id2 = UniqueId();

     for (std::size_t i = 0; i < id1.size(); ++i) {
        if (id1[i] != id2[i]) {
           return true;
        }
     }

     return false;
  }(),
  "Multiple calls to UniqueId should yield different values"
);

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
DeobfuscateString(std::vector<char> const& obfuscated) {
   std::string result{};
   result.resize(obfuscated.size());
   std::cout << KEY.value_.size() << std::endl;
   for (std::size_t i = 0; i < result.size(); ++i) {
      result[i] = obfuscated[i] ^ KEY.value_[i % KEY.value_.size()];
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
