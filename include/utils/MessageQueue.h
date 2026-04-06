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

template <bool THROWS = false>
class MessageQueue : public Pool<THROWS, 1> {
public:
   using Pool<THROWS, 1>::Pool;
   ~MessageQueue() override = default;

   using Pool<THROWS, 1>::Dispatch;
   using Pool<THROWS, 1>::Stop;

   template <class...>
      requires(!THROWS)
   [[nodiscard]] bool Ensure(std::function<void()>&& func) const noexcept;

   template <class...>
      requires(THROWS)
   void Ensure(std::function<void()>&& func) const noexcept(false);

   std::thread::id ThreadId() const;

private:
   template <bool MAIN = true>
   class Dispatcher {
   public:
      Dispatcher(MessageQueue& queue)
         : self_{queue} {}

      template <class...>
         requires(!MAIN)
      bool await_ready() const {
         return std::this_thread::get_id() == self_.ThreadId();
      }

      template <class...>
         requires(!MAIN)
      bool await_suspend(std::coroutine_handle<> h) const {
         if constexpr (THROWS) {
            success_ = false;
            self_.Dispatch([h] constexpr { h.resume(); });
            success_ = true;
            return true;
         } else {
            success_ = self_.Dispatch([h] constexpr { h.resume(); });
            return success_;
         }
      }

      template <class...>
         requires(!MAIN)
      void await_resume() const noexcept(false) {
         if (!success_) {
            throw std::runtime_error("Pool thread stopped while waiting for event");
         }
      }

      template <class...>
         requires(MAIN)
      Dispatcher<false> operator()() const {
         return Dispatcher<false>{self_};
      }

   private:
      MessageQueue& self_;
      mutable bool  success_{true};
   };

public:
   Dispatcher<true> ensure_{*this};
};