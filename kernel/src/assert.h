#pragma once

#include "common.h"

extern "C" {
void __attribute__((noreturn)) crash();
void __attribute__((noreturn)) _assert_fail(const char* file, const char* func, int line,
		const char* expr, uint8_t assertId=0);
}

#define assert_2(b, id) do { if(!(b)) { _assert_fail(__FILE__, __FUNCTION__, \
		__LINE__, #b, id); while(true);} } while(0)
#define assert_1(b) assert_2(b,0)

#define assert(...) CONCAT(assert_, VARGS(__VA_ARGS__))(__VA_ARGS__)

//#define assert(b, id) _assert(b, __FILE__, __FUNCTION__, __LINE__, #b, id)
