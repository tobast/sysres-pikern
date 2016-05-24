#pragma once

#include "common.h"

void crash();
void _assert(bool b, const char* file, const char* func, int line,
		const char* expr, uint8_t assertId=0);

#define assert(b, id) _assert(b, __FILE__, __FUNCTION__, __LINE__, #b, id)

