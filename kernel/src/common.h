#pragma once

#include <cstdint>
#include <cstddef>

typedef int32_t s32;
typedef uint32_t u32;
typedef int16_t s16;
typedef uint16_t u16;
typedef int8_t s8;
typedef uint8_t u8;

inline u32 min(u32 a, u32 b) {
	return (a < b) ? a : b;
}

inline u32 max(u32 a, u32 b) {
	return (a > b) ? a : b;
}
