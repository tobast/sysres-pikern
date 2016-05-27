#pragma once

#include "common.h"

void crash();
void _assert(bool b, const char* file, const char* func, int line,
		const char* expr, uint8_t assertId=0);

#define assert_2(b, id) do {_assert(b, __FILE__, __FUNCTION__, __LINE__, #b, id); while(true); } while(0)
#define assert_1(b) do {_assert(b, __FILE__, __FUNCTION__, __LINE__, #b, 0); while(true); } while(0)

#define assert(...) CONCAT(assert_, VARGS(__VA_ARGS__))(__VA_ARGS__)
