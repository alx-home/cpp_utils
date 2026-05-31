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

#include <array>
#include <condition_variable>
#include <functional>
#include <list>
#include <map>
#include <optional>
#include <string_view>
#include <thread>
#include <type_traits>
#include <variant>

class QueueStopped : public std::runtime_error {
public:
   QueueStopped(std::string_view name)
      : std::runtime_error(std::string("Pool '") + std::string{name} + "' is stopped!") {}
};

template <bool THROWS = false, std::size_t SIZE = 10>
class Pool {
public:
   using time_point = std::chrono::steady_clock::time_point;
   using duration   = std::chrono::steady_clock::duration;

   explicit Pool(std::string_view thread_name);
   virtual ~Pool();

   std::string_view GetName() const noexcept { return name_; }

   template <class FUN>
   [[nodiscard]] std::
     conditional_t<THROWS, void, std::pair<bool, std::optional<std::move_only_function<void()>>>>
     Dispatch(
       FUN&&                                             func,
       std::optional<std::variant<time_point, duration>> delay = std::nullopt
     ) const noexcept(!THROWS) {
      auto dispatch =
        [this, func = std::forward<FUN>(func)](std::optional<time_point> delay) mutable constexpr {
           if constexpr (std::is_same_v<std::function<void()>, std::decay_t<FUN>>) {
              return DispatchImpl(
                std::move_only_function<void()>{[func = std::move(func)]() mutable { func(); }},
                std::move(delay)
              );
           } else {
              return DispatchImpl(std::move(func), std::move(delay));
           }
        };

      if (delay) {
         if (std::holds_alternative<duration>(*delay)) {
            return dispatch(std::chrono::steady_clock::now() + std::get<duration>(*delay));
         } else {
            return dispatch(std::get<time_point>(*delay));
         }
      } else {
         return dispatch(std::nullopt);
      }
   }

   std::array<std::thread::id, SIZE> ThreadIds() const;

   void Stop();

private:
   template <class...>
      requires(!THROWS)
   [[nodiscard]] std::pair<bool, std::optional<std::move_only_function<void()>>> DispatchImpl(
     std::move_only_function<void()>&&,
     std::optional<time_point> delay = std::nullopt
   ) const noexcept;

   template <class...>
      requires(THROWS)
   void DispatchImpl(
     std::move_only_function<void()>&&,
     std::optional<time_point> delay = std::nullopt
   ) const noexcept(false);

private:
   std::string                                        name_{};
   std::atomic<bool>                                  running_{true};
   std::atomic<bool>                                  stopping_{false};
   mutable std::list<std::move_only_function<void()>> queue_{};

   mutable std::condition_variable cv_{};
   mutable std::mutex              mutex_{};

   mutable time_point                                            next_event_{time_point::max()};
   mutable std::map<time_point, std::move_only_function<void()>> delayed_events_{};

   struct Threads {
      mutable std::condition_variable dispatch_cv_{};
      std::jthread                    dispatcher_{};
      std::array<std::jthread, SIZE>  workers_{};
      std::atomic<std::size_t>        done_{0};
   };

   std::conditional_t<SIZE == 1, std::jthread, Threads> threads_{};
};

#include "Pool.inl"
