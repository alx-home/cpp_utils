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

#include <condition_variable>
#include <coroutine>
#include <functional>
#include <map>
#include <optional>
#include <string_view>

template <std::size_t SIZE = 10>
class Poll {
public:
   explicit Poll(std::string_view thread_name);
   virtual ~Poll();

   using time_point = std::chrono::steady_clock::time_point;
   [[nodiscard]] bool
   Dispatch(std::function<void()>&&, std::optional<time_point> delay = std::nullopt);
   using duration = std::chrono::steady_clock::duration;
   [[nodiscard]] bool Dispatch(std::function<void()>&&, duration delay);

   std::array<std::thread::id, SIZE> ThreadIds() const;

private:
   template <bool MAIN = true>
   class Dispatcher {
   public:
      Dispatcher(Poll& queue, std::optional<time_point> until = std::nullopt)
         : self_{queue}
         , until_{until} {}

      template <class...>
         requires(!MAIN)
      bool await_ready() const {
         return false;
      }

      template <class...>
         requires(!MAIN)
      bool await_suspend(std::coroutine_handle<> h) const {
         success_ = self_.Dispatch([h] constexpr { h.resume(); }, until_);
         return success_;
      }

      template <class...>
         requires(!MAIN)
      void await_resume() const noexcept(false) {
         if (!success_) {
            throw std::runtime_error("Poll thread stopped while waiting for event");
         }
      }

      Dispatcher<false> operator()(std::optional<time_point> until = std::nullopt) const {
         return Dispatcher<false>{self_, until};
      }
      Dispatcher<false> operator()(duration delay) const {
         return Dispatcher<false>{self_, std::chrono::steady_clock::now() + delay};
      }

   private:
      Poll&                     self_;
      std::optional<time_point> until_{std::nullopt};
      mutable bool              success_{false};
   };

public:
   Dispatcher<true> dispatch_{*this};

private:
   bool                             running_{true};
   std::list<std::function<void()>> queue_{};
   std::condition_variable          cv_{};
   std::mutex                       mutex_{};

   time_point                                  next_event_{time_point::max()};
   std::map<time_point, std::function<void()>> delayed_events_{};

   std::array<std::jthread, SIZE> threads_;
};

#include "Poll.inl"