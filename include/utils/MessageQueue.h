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

#include "Pool.h"
#include <functional>
#include <type_traits>

template <bool THROWS = false>
class MessageQueue : public Pool<THROWS, 1> {
public:
   using Pool<THROWS, 1>::Pool;
   ~MessageQueue() override = default;

   using Pool<THROWS, 1>::Dispatch;
   using Pool<THROWS, 1>::Stop;

   std::thread::id ThreadId() const;

   template <class FUN>
   [[nodiscard]] std::
     conditional_t<THROWS, void, std::pair<bool, std::optional<std::move_only_function<void()>>>>
     Ensure(FUN&& func) const noexcept(!THROWS) {
      if constexpr (std::is_same_v<std::function<void()>, std::decay_t<FUN>>) {
         return EnsureImpl(std::move_only_function<void()>{[func = std::move(func)]() mutable {
            func();
         }});
      } else {
         return EnsureImpl(std::move(func));
      }
   }

private:
   template <class...>
      requires(!THROWS)
   [[nodiscard]] std::pair<bool, std::optional<std::move_only_function<void()>>> EnsureImpl(
     std::move_only_function<void()>&& func
   ) const noexcept;

   template <class...>
      requires(THROWS)
   void EnsureImpl(std::move_only_function<void()>&& func) const noexcept(false);
};
