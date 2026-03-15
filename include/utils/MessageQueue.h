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

class MessageQueue {
public:
   explicit MessageQueue(std::string_view thread_name);
   virtual ~MessageQueue();

   using time_point = std::chrono::steady_clock::time_point;
   bool Dispatch(std::function<void()>&&, std::optional<time_point> delay = std::nullopt);
   using duration = std::chrono::steady_clock::duration;
   bool Dispatch(std::function<void()>&&, duration delay);

private:
   class Dispatcher {
   public:
      Dispatcher(MessageQueue& queue, std::optional<time_point> until = std::nullopt)
         : queue_{queue}
         , until_{until} {}

      bool await_ready() const { return false; }
      auto await_suspend(std::coroutine_handle<> h) const {
         queue_.Dispatch([&h] constexpr { h.resume(); }, until_);
      }
      void await_resume() const noexcept(false) {}

      Dispatcher operator()(time_point until) const { return Dispatcher{queue_, until}; }
      Dispatcher operator()(duration delay) const {
         return Dispatcher{queue_, std::chrono::steady_clock::now() + delay};
      }

   private:
      MessageQueue&             queue_;
      std::optional<time_point> until_{std::nullopt};
   };

public:
   Dispatcher dispatch_{*this};

   std::thread::id ThreadId() const;

private:
   bool                             runing_{true};
   std::list<std::function<void()>> queue_{};
   std::condition_variable          cv_{};
   std::mutex                       mutex_{};

   time_point                                  next_event_{time_point::max()};
   std::map<time_point, std::function<void()>> delayed_events_{};

   std::jthread thread_;
};