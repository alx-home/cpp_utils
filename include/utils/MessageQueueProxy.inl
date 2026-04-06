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

#include <coroutine>
#include <functional>
#include <stdexcept>
#include <thread>
#include <type_traits>

namespace promise::details {
template <class>
class WPromise;
}

template <class T>
using WPromise = promise::details::WPromise<T>;

namespace utils::queue {

template <class OBJECT, class OBJECT_PUBLIC = OBJECT>
class Proxy {
public:
   template <class... ARGS>
   Proxy(ARGS&&... args)
      : details_(std::forward<ARGS>(args)...) {}
   ~Proxy() = default;

   [[nodiscard]] auto operator()(std::function<void(OBJECT_PUBLIC&)>&& callback) noexcept {
      using Return = decltype(details_.MessageQueue::Ensure([] constexpr {}));
      if constexpr (std::is_void_v<Return>) {
         details_.MessageQueue::Ensure([this, callback = std::move(callback)] constexpr mutable {
            callback(details_);
         });
      } else {
         return details_.MessageQueue::Ensure(
           [this, callback = std::move(callback)] constexpr mutable { callback(details_); }
         );
      }
   }

   template <class T>
      requires(!std::is_void_v<T>)
   WPromise<T> operator()(std::function<T(OBJECT_PUBLIC&)>&& callback);

   bool await_ready() const {
      return std::this_thread::get_id() == details_.MessageQueue::ThreadId();
   }

   bool await_suspend(std::coroutine_handle<> h) const {
      if (!details_.MessageQueue::Ensure([h] constexpr { h.resume(); })) {
         throw std::runtime_error("Failed to dispatch coroutine: MessageQueue is stopped");
      }
      return true;
   }

   OBJECT_PUBLIC& await_resume() noexcept(false) {
      assert(std::this_thread::get_id() == details_.MessageQueue::ThreadId());
      return details_;
   }

   OBJECT_PUBLIC const& await_resume() const noexcept(false) {
      assert(std::this_thread::get_id() == details_.MessageQueue::ThreadId());
      return details_;
   }

private:
   OBJECT details_{};
};

}  // namespace utils::queue