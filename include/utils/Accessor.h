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

#include <utility>

namespace accessor {

enum class Edit { PUBLIC, PROTECTED, PRIVATE };

template <class PARENT, class TYPE, Edit VISIBILITY = Edit::PRIVATE>
class Member {
public:
   constexpr Member(
     TYPE value                           = {},
     TYPE const& (*getter)(Member const&) = [](Member const& self) constexpr -> TYPE const& {
        return self.value_;
     },
     void (*setter)(Member&, TYPE) =
       [](Member& self, TYPE value) constexpr { self.value_ = std::move(value); }
   )
      : value_(std::move(value))
      , getter_(getter)
      , setter_(setter) {}

   explicit(false) operator TYPE const&() const { return this->value_; }
   TYPE const& operator*() const { return this->value_; }

   template <class SELF, class TYPE2>
      requires(VISIBILITY == Edit::PUBLIC)
   Member<PARENT, TYPE, VISIBILITY>& operator=(this SELF&& self, TYPE2&& elem) {
      self.value_ = std::forward<TYPE2>(elem);

      return self;
   }

   template <class...>
      requires(VISIBILITY == Edit::PUBLIC)
   explicit(false) operator TYPE&() {
      return this->value_;
   }

   template <class...>
      requires(VISIBILITY == Edit::PUBLIC)
   TYPE& operator*() {
      return this->value_;
   }

   template <class...>
      requires(VISIBILITY == Edit::PUBLIC)
   TYPE* operator->() {
      return &this->value_;
   }

protected:
   template <class SELF, class TYPE2>
      requires(VISIBILITY == Edit::PROTECTED)
   Member<PARENT, TYPE, VISIBILITY>& operator=(this SELF&& self, TYPE2&& elem) {
      self.value_ = std::forward<TYPE2>(elem);

      return self;
   }

   template <class...>
      requires(VISIBILITY == Edit::PROTECTED)
   explicit(false) operator TYPE&() {
      return this->value_;
   }

   template <class...>
      requires(VISIBILITY == Edit::PROTECTED)
   TYPE& operator*() {
      return this->value_;
   }

   template <class...>
      requires(VISIBILITY == Edit::PROTECTED)
   TYPE* operator->() {
      return &this->value_;
   }

private:
   template <class SELF, class TYPE2>
      requires(VISIBILITY == Edit::PRIVATE)
   Member<PARENT, TYPE, VISIBILITY>& operator=(this SELF&& self, TYPE2&& elem) {
      self.value_ = std::forward<TYPE2>(elem);

      return self;
   }

   template <class...>
      requires(VISIBILITY == Edit::PRIVATE)
   explicit(false) operator TYPE&() {
      return this->value_;
   }

   template <class...>
      requires(VISIBILITY == Edit::PRIVATE)
   TYPE& operator*() {
      return this->value_;
   }

   template <class...>
      requires(VISIBILITY == Edit::PRIVATE)
   TYPE* operator->() {
      return &this->value_;
   }

   TYPE value_;
   TYPE const& (*getter_)(Member const&);
   void (*setter_)(Member&, TYPE);

   friend PARENT;
};

}  // namespace accessor

template <class PARENT, class TYPE, accessor::Edit VISIBILITY = accessor::Edit::PRIVATE>
using SGet = accessor::Member<PARENT, TYPE, VISIBILITY>;

using Access = accessor::Edit;