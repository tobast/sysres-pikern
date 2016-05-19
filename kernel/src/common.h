#pragma once

#include <cstdint>
#include <cstddef>

typedef int64_t s64;
typedef uint64_t u64;
typedef int32_t s32;
typedef uint32_t u32;
typedef int16_t s16;
typedef uint16_t u16;
typedef int8_t s8;
typedef uint8_t u8;

template<typename T>
inline T min(T a, T b) {
	return (a < b) ? a : b;
}

template<typename T>
inline T max(T a, T b) {
	return (a > b) ? a : b;
}

// Networking
typedef uint32_t Ipv4Addr;
typedef uint64_t HwAddr;

// String
inline unsigned str_len(const char* s) {
	for(unsigned pos=0; ; pos++) {
		if(s[pos] == '\0')
			return pos;
	}
	return -1; // to please g++
}

