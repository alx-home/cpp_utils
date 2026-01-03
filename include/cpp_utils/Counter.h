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

#include <cstddef>

/**
 * @file Counter.h
 * @brief Provides a compile-time counter utility using template metaprogramming.
 *
 * This header defines the `cpp_utils::Counter` function template and supporting
 * structures to generate unique compile-time values (IDs) in a constexpr context.
 * The mechanism leverages SFINAE, friend injection, and template instantiation tricks
 * to incrementally produce unique integer values at compile time.
 *
 * Usage:
 *   - Call `cpp_utils::Counter()` in a constexpr context to obtain a unique integer ID.
 *   - Each invocation in a different context yields a distinct value.
 *
 * Components:
 *   - `CounterHelper<ID>`: Helper struct for SFINAE-based detection of defined IDs.
 *   - `Counter()`: Constexpr function template that returns the next available unique ID.
 *
 * @note This utility is useful for generating unique type or value tags at compile time,
 *       such as for static registration, metaprogramming, or type-safe counters.
 */
namespace cpp_utils {

/**
 * @brief Helper struct template for compile-time counter implementation.
 *
 * This struct is used to detect and manage unique compile-time identifiers using SFINAE and friend
 * function injection. It provides mechanisms to check if a counter value (ID) has been defined and
 * to trigger its definition.
 *
 * @tparam ID The unique compile-time identifier for the counter.
 * @tparam Unused for reusing CounterHelper in different contexts. Defaults to void for Counter()
 * function.
 *
 * Members:
 * - tag: Alias for the current CounterHelper specialization, used for SFINAE detection.
 * - Generator: Nested struct that, when instantiated, defines a friend consteval function IsDefined
 *   for the given tag.
 * - Exists: Static consteval function template that checks if IsDefined is available for the tag.
 *           Returns true if defined, otherwise triggers Generator instantiation and returns false.
 */
template <auto ID, typename = void>
struct CounterHelper {
   using tag = CounterHelper;

   struct Generator {
      friend consteval auto IsDefined(tag) { return true; }
   };
   // Friend declaration to allow SFINAE detection
   friend consteval auto IsDefined(tag);

   template <typename TAG = tag, auto = IsDefined(TAG{})>
   static consteval auto Exists(std::size_t) {
      return true;
   }

   // Fallback overload (int preferred over float in overload resolution)
   static consteval auto Exists(float) {
      // Trigger instantiation of the Generator to define IsDefined
      Generator();
      return false;
   }
};

/**
 * @brief Compile-time counter function for generating unique integral values.
 *
 * This function template recursively increments the counter until it finds an unused ID,
 * as determined by the CounterHelper<ID>::Exists(ID) trait. It is intended for use in
 * metaprogramming scenarios where unique compile-time values are needed, such as for
 * tagging or type registration.
 *
 * @tparam ID The starting value for the counter (default is 0).
 * @tparam TAG A tag type for reusing Counter in different contexts. Defaults to void.
 * function.
 * @tparam (anonymous) An unnamed template parameter used to force template re-instantiation.
 * @return The first unused integral ID at compile time.
 */
template <
  std::size_t ID = 0,
  typename TAG   = void,
  class FUN      = decltype([]() consteval { /* Force template re-instantiation */ })>
consteval auto
Counter() {
   if constexpr (CounterHelper<ID, TAG>::Exists(ID)) {
      return Counter<ID + 1, TAG>();
   } else {
      return ID;
   }
}

struct CounterTagAssert;
// Ensure that multiple calls yield different values
static_assert(
  Counter<0, CounterTagAssert>() != Counter<0, CounterTagAssert>(),
  "Multiple calls to Counter should yield different values"
);
}  // namespace cpp_utils