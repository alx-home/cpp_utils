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

#include "Poll.h"

class MessageQueue : public Poll<1> {
public:
   using Poll<1>::Poll;
   ~MessageQueue() override = default;

   using Poll<1>::Dispatch;
   using Poll<1>::Stop;

   [[nodiscard]] bool Ensure(std::function<void()>&& func) const;

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
         success_ = self_.Dispatch([h] constexpr { h.resume(); });
         return success_;
      }

      template <class...>
         requires(!MAIN)
      void await_resume() const noexcept(false) {
         if (!success_) {
            throw std::runtime_error("Poll thread stopped while waiting for event");
         }
      }

      Dispatcher<false> operator()() const { return Dispatcher<false>{self_}; }

   private:
      MessageQueue& self_;
      mutable bool  success_{true};
   };

public:
   Dispatcher<true> ensure_{*this};
};