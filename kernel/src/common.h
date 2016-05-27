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

// For macro with default arguments
// From http://stackoverflow.com/questions/27049491/can-c-c-preprocessor-macros-have-default-parameter-values
#define VARGS_(_10, _9, _8, _7, _6, _5, _4, _3, _2, _1, N, ...) N 
#define VARGS(...) VARGS_(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define CONCAT_(a, b) a##b
#define CONCAT(a, b) CONCAT_(a, b)
