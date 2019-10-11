/**
 * MIT License
 *
 * Copyright (c) 2019 Christoffer Gustafsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef TYPES_HPP_
#define TYPES_HPP_

#include <cstdint>

using char8 = char;
static_assert(sizeof(char8) == 1, "char8 is not 8-bits large");

// ============================================================ //

using s8 = int8_t;
using u8 = uint8_t;

using s16 = int16_t;
using u16 = uint16_t;

using s32 = int32_t;
using u32 = uint32_t;

using s64 = int64_t;
using u64 = uint64_t;

// ============================================================ //

using s8_fast = int_fast8_t;
using u8_fast = uint_fast8_t;

using s16_fast = int_fast16_t;
using u16_fast = uint_fast16_t;

using s32_fast = int_fast32_t;
using u32_fast = uint_fast32_t;

using s64_fast = int_fast64_t;
using u64_fast = uint_fast64_t;

// ============================================================ //

using f32 = float;
static_assert(sizeof(f32) == 4, "f32 is not 32-bits large");

using f64 = double;
static_assert(sizeof(f64) == 8, "f64 is not 64-bits large");

// ============================================================ //

#endif  // TYPES_HPP_
