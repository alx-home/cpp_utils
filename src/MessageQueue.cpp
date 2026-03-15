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
#include "utils/String.h"

#include <processthreadsapi.h>

MessageQueue::MessageQueue(std::string_view thread_name)
   : thread_(
       [this](std::string_view thread_name) constexpr {
          SetThreadDescription(
            GetCurrentThread(), utils::WidenString(std::move(thread_name)).c_str()
          );

          while (true) {
             std::unique_lock lock{mutex_};

             if (delayed_events_.size()) {
                // Move delayed events that are due to the main queue

                auto const upper_bound =
                  delayed_events_.upper_bound(std::chrono::steady_clock::now());
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
             }

             if (queue_.size()) {
                auto elem = std::move(queue_.front());
                queue_.pop_front();
                lock.unlock();

                elem();
             } else if (runing_) {
                cv_.wait_until(lock, next_event_);
             } else {
                break;
             }
          }
       },
       std::move(thread_name)
     ) {}

MessageQueue::~MessageQueue() {
   {
      std::unique_lock lock{mutex_};
      runing_ = false;
      cv_.notify_all();
   }
   thread_.join();
}

bool
MessageQueue::Dispatch(std::function<void()>&& func, std::optional<time_point> delay) {
   std::unique_lock lock{mutex_};
   if (runing_) {
      if (delay) {
         delayed_events_.emplace(*delay, std::move(func));
         next_event_ = std::min(next_event_, delayed_events_.begin()->first);
      } else {
         queue_.emplace_back(std::move(func));
      }
      cv_.notify_all();
      return true;
   }

   return false;
}

bool
MessageQueue::Dispatch(std::function<void()>&& func, duration delay) {
   return Dispatch(std::move(func), std::chrono::steady_clock::now() + delay);
}

std::thread::id
MessageQueue::ThreadId() const {
   return thread_.get_id();
}