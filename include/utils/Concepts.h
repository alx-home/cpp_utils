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

#include <type_traits>

namespace alx::utils {

template <typename FUNC>
concept _function = requires(FUNC func) { func(); };

/**
 * @brief Extract a normalized function signature from callable types.
 *
 * The extracted signature is always represented as RETURN(ARGS...).
 */
template <class T>
struct Signature;

/**
 * @brief Signature passthrough for plain function types.
 */
template <class T, class... ARGS>
struct Signature<T(ARGS...)> {
   using type = T(ARGS...);
};

/**
 * @brief Signature extraction for function pointers.
 */
template <class T, class... ARGS>
struct Signature<T (*)(ARGS...)> {
   using type = T(ARGS...);
};

/**
 * @brief Signature extraction for function references.
 */
template <class T, class... ARGS>
struct Signature<T (&)(ARGS...)> {
   using type = T(ARGS...);
};

/**
 * @brief Signature extraction for callable objects exposing operator().
 *
 * Resolves the operator() member pointer first, then reuses member-function
 * specializations below.
 */
template <class T>
   requires requires { &T::operator(); }
struct Signature<T> {
   using type = typename Signature<decltype(&T::operator())>::type;
};

/**
 * @brief Signature extraction for non-const member function pointers.
 */
template <class OBJ, class RETURN, class... ARGS>
struct Signature<RETURN (OBJ::*)(ARGS...)> {
   using type = RETURN(ARGS...);
};

/**
 * @brief Signature extraction for const member function pointers.
 */
template <class OBJ, class RETURN, class... ARGS>
struct Signature<RETURN (OBJ::*)(ARGS...) const> {
   using type = RETURN(ARGS...);
};

/**
 * @brief Get the signature of a callable.
 */
template <class FUN>
using signature_t = typename Signature<std::remove_cvref_t<FUN>>::type;

}  // namespace alx::utils
