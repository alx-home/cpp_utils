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

#include "Counter.h"
#include "utils/String.h"
#include <iostream>
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
 * @brief Combines two hash values into a single hash value.
 *
 * This function takes two hash values and combines them using a specific
 * mixing algorithm to produce a new hash value. It is useful for
 * combining the hashes of multiple fields when implementing hash functions
 * for composite types.
 *
 * @param h The first hash value.
 * @param v The second hash value to combine with the first.
 * @return The combined hash value.
 */
consteval std::size_t
HashCombine(std::size_t h, std::size_t v) {
   constexpr std::size_t k = 0x9e3779b97f4a7c15ULL;
   return h ^ (v + k + (h << 6) + (h >> 2));
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
template <
  std::size_t SIZE = 1,
  std::size_t ID   = 0,
  class FUN        = decltype([]() consteval { /* Force template re-instantiation */ })>
consteval auto
UniqueId() {
   if constexpr (CounterHelper<ID, UniqueIdTag>::Exists(ID)) {
      return UniqueId<SIZE, ID + 1>();
   } else {
      std::array<char, 9> time{__TIME__};

      std::array<char, 8 * SIZE> result{};

      std::size_t seed =
        (static_cast<std::size_t>(time[0]) << 40 | static_cast<std::size_t>(time[1]) << 32
         | static_cast<std::size_t>(time[3]) << 24 | static_cast<std::size_t>(time[4]) << 16
         | static_cast<std::size_t>(time[6]) << 8 | static_cast<std::size_t>(time[7]))
        + ID * SIZE;

      for (std::size_t j = 0; j < SIZE; ++j) {
         std::size_t h = seed + j;
         for (char i : time) {
            h = HashCombine(h, static_cast<std::size_t>(i));
         }

         result[j * 8 + 0] = static_cast<char>(h & 0xFF);
         result[j * 8 + 1] = static_cast<char>((h >> 8) & 0xFF);
         result[j * 8 + 2] = static_cast<char>((h >> 16) & 0xFF);
         result[j * 8 + 3] = static_cast<char>((h >> 24) & 0xFF);
         result[j * 8 + 4] = static_cast<char>((h >> 32) & 0xFF);
         result[j * 8 + 5] = static_cast<char>((h >> 40) & 0xFF);
         result[j * 8 + 6] = static_cast<char>((h >> 48) & 0xFF);
         result[j * 8 + 7] = static_cast<char>((h >> 56) & 0xFF);
      }

      return result;
   }
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
