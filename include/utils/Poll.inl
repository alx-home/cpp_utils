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

#include "utils/Poll.h"
#include "utils/String.h"

#include <processthreadsapi.h>
#include <algorithm>
#include <cassert>

template <std::size_t SIZE>
Poll<SIZE>::Poll(std::string_view thread_name) {
   for (std::size_t i = 0; i < SIZE; ++i) {
      threads_[i] = std::jthread{
        [this](std::string&& thread_name) constexpr {
           SetThreadDescription(
             GetCurrentThread(), utils::WidenString(std::move(thread_name)).c_str()
           );

           while (true) {
              std::unique_lock lock{mutex_};

              if (delayed_events_.size()) {
                 // Move delayed events that are due to the main queue

                 auto const upper_bound =
                   running_ ? delayed_events_.upper_bound(std::chrono::steady_clock::now())
                            : delayed_events_.end();
                 for (auto it = delayed_events_.begin(); it != upper_bound;) {
                    auto func = std::move(it->second);
                    queue_.emplace_back(std::move(func));
                    it = delayed_events_.erase(it);
                 }

                 if (!delayed_events_.empty()) {
                    next_event_ = std::min(next_event_, delayed_events_.begin()->first);
                 } else {
                    next_event_ = time_point::max();
                 }

                 cv_.notify_all();
              }

              if (queue_.size()) {
                 auto elem = std::move(queue_.front());
                 queue_.pop_front();
                 lock.unlock();

                 elem();
              } else if (running_) {
#ifndef NDEBUG
                 SetThreadDescription(
                   GetCurrentThread(),
                   (utils::WidenString(std::move(thread_name)) + L" IDL").c_str()
                 );
#endif
                 cv_.wait_until(lock, next_event_);
#ifndef NDEBUG
                 SetThreadDescription(
                   GetCurrentThread(), utils::WidenString(std::move(thread_name)).c_str()
                 );
#endif
              } else {
                 assert(delayed_events_.empty());
                 break;
              }
           }
        },
        [i, &thread_name] constexpr -> std::string {
           if constexpr (SIZE == 1) {
              auto const _{i};
              return std::string{thread_name};
           } else {
              return std::string{thread_name} + " #" + std::to_string(i);
           }
        }()
      };
   }
}

template <std::size_t SIZE>
Poll<SIZE>::~Poll() {
   {
      std::unique_lock lock{mutex_};
      running_ = false;
      cv_.notify_all();
   }

   for (auto& thread : threads_) {
      thread.join();
   }
}

template <std::size_t SIZE>
bool
Poll<SIZE>::Dispatch(std::function<void()>&& func, std::optional<time_point> delay) {
   std::unique_lock lock{mutex_};
   if (running_) {
      if (delay) {
         delayed_events_.emplace(*delay, std::move(func));
         next_event_ = std::min(next_event_, delayed_events_.begin()->first);
         cv_.notify_all();
      } else {
         queue_.emplace_back(std::move(func));
         cv_.notify_one();
      }

      return true;
   }

   return false;
}

template <std::size_t SIZE>
bool
Poll<SIZE>::Dispatch(std::function<void()>&& func, duration delay) {
   return Dispatch(std::move(func), std::chrono::steady_clock::now() + delay);
}

template <std::size_t SIZE>
std::array<std::thread::id, SIZE>
Poll<SIZE>::ThreadIds() const {
   std::array<std::thread::id, SIZE> ids;
   std::ranges::transform(threads_, ids.begin(), [](const std::jthread& thread) {
      return thread.get_id();
   });
   return ids;
}