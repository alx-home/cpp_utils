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

#include "utils/Pool.inl"

#define POOL_IMPL(IDX)                                                                           \
   template class Pool<true, IDX>;                                                               \
   template void Pool<true, IDX>::Dispatch<>(                                                    \
     std::function<void()>&&, std::optional<Pool<true, IDX>::time_point>                         \
   ) const noexcept(false);                                                                      \
   template void Pool<true, IDX>::Dispatch<>(std::function<void()>&&, Pool<true, IDX>::duration) \
     const noexcept(false);                                                                      \
                                                                                                 \
   template class Pool<false, IDX>;                                                              \
   template std::pair<bool, std::optional<std::function<void()>>> Pool<false, IDX>::Dispatch<>(  \
     std::function<void()>&&, std::optional<Pool<false, IDX>::time_point>                        \
   ) const noexcept;                                                                             \
   template std::pair<bool, std::optional<std::function<void()>>> Pool<false, IDX>::Dispatch<>(  \
     std::function<void()>&&, Pool<false, IDX>::duration                                         \
   ) const noexcept;

#define POOL_IMPL_10(BASE) \
   POOL_IMPL((BASE) + 1)   \
   POOL_IMPL((BASE) + 2)   \
   POOL_IMPL((BASE) + 3)   \
   POOL_IMPL((BASE) + 4)   \
   POOL_IMPL((BASE) + 5)   \
   POOL_IMPL((BASE) + 6)   \
   POOL_IMPL((BASE) + 7)   \
   POOL_IMPL((BASE) + 8)   \
   POOL_IMPL((BASE) + 9)   \
   POOL_IMPL((BASE) + 10)

#define POOL_IMPL_100(BASE)  \
   POOL_IMPL_10((BASE) + 0)  \
   POOL_IMPL_10((BASE) + 10) \
   POOL_IMPL_10((BASE) + 20) \
   POOL_IMPL_10((BASE) + 30) \
   POOL_IMPL_10((BASE) + 40) \
   POOL_IMPL_10((BASE) + 50) \
   POOL_IMPL_10((BASE) + 60) \
   POOL_IMPL_10((BASE) + 70) \
   POOL_IMPL_10((BASE) + 80) \
   POOL_IMPL_10((BASE) + 90)

POOL_IMPL_100(0)
POOL_IMPL_100(100)

#undef POOL_IMPL_100
#undef POOL_IMPL_10
#undef POOL_IMPL