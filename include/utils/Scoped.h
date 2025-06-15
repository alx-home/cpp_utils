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

#include "Concepts.h"
#include <type_traits>

template <_function FUNC>
struct ScopeExit;

template <_function FUNC>
   requires(std::is_reference_v<FUNC>)
struct ScopeExit<FUNC> {
   FUNC const& do_;

   explicit constexpr ScopeExit(FUNC const& do_)
      : do_(do_) {}

   constexpr ~ScopeExit() { do_(); }
};

template <_function FUNC>
   requires(!std::is_reference_v<FUNC>)
struct ScopeExit<FUNC> {
   FUNC do_;

   constexpr explicit ScopeExit(FUNC do_)
      : do_(std::move(do_)) {}

   constexpr ~ScopeExit() { do_(); }
};

template <_function FUNC>
ScopeExit(FUNC) -> ScopeExit<FUNC>;