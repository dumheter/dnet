#ifndef DNET_TYPES_HPP
#define DNET_TYPES_HPP

// ====================================================================== //
// Headers
// ====================================================================== //

#include <cstdint>

// ====================================================================== //
// Type Aliases
// ====================================================================== //

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

using f32 = float;
static_assert(sizeof(f32) == 4, "f32 is not 32-bits large");

using f64 = double;
static_assert(sizeof(f64) == 8, "f64 is not 64-bits large");

// ============================================================ //

#endif //DNET_TYPES_HPP
