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

#include "utils/MessageQueue.h"

template <bool THROWS>
std::thread::id
MessageQueue<THROWS>::ThreadId() const {
   return Pool<THROWS, 1>::ThreadIds()[0];
}

template <bool THROWS>
template <class...>
   requires(!THROWS)
std::pair<bool, std::optional<std::function<void()>>>
MessageQueue<THROWS>::Ensure(std::function<void()>&& func) const noexcept {
   if (std::this_thread::get_id() == ThreadId()) {
      func();
      return {true, std::nullopt};
   }

   return Dispatch(std::move(func));
}

template <bool THROWS>
template <class...>
   requires(THROWS)
void
MessageQueue<THROWS>::Ensure(std::function<void()>&& func) const noexcept(false) {
   if (std::this_thread::get_id() == ThreadId()) {
      func();
      return;
   }

   Dispatch(std::move(func));
}

template class MessageQueue<false>;
template class MessageQueue<true>;

template void MessageQueue<true>::Ensure<>(std::function<void()>&&) const noexcept(false);
template std::pair<bool, std::optional<std::function<void()>>> MessageQueue<false>::Ensure<>(
  std::function<void()>&&
) const noexcept;