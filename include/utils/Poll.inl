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

template <std::size_t SIZE>
Poll<SIZE>::Poll(std::string_view thread_name) {
   for (std::size_t i = 0; i < SIZE; ++i) {
      threads_[i] = std::jthread{
        [this, i](std::string&& thread_name) constexpr {
           SetThreadDescription(
             GetCurrentThread(), utils::WidenString(std::move(thread_name)).c_str()
           );

           while (true) {
              std::unique_lock lock{mutex_};

              if (queue_[i].size()) {
                 auto elem = std::move(queue_[i].front());
                 queue_[i].pop_front();
                 lock.unlock();

                 elem();
              } else if (runing_) {
                 cv_.wait(lock);
              } else {
                 break;
              }
           }
        },
        std::string{thread_name} + " #" + std::to_string(i)
      };
   }
}

template <std::size_t SIZE>
Poll<SIZE>::~Poll() {
   {
      std::unique_lock lock{mutex_};
      runing_ = false;
      cv_.notify_all();
   }
   for (auto& thread : threads_) {
      thread.join();
   }
}

template <std::size_t SIZE>
bool
Poll<SIZE>::Dispatch(std::function<void()> func) {
   std::unique_lock lock{mutex_};
   if (runing_) {
      auto        queue    = queue_.begin();
      std::size_t min_size = queue->size();

      for (auto it = std::next(queue); it != queue_.end(); ++it) {
         if (it->size() < min_size) {
            queue    = it;
            min_size = it->size();
         }
      }
      queue->emplace_back(std::move(func));
      cv_.notify_all();
      return true;
   }

   return false;
}