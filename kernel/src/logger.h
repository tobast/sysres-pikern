#pragma once

#include "networkCore.h"
#include <cstdarg>

void appendLog(const char* source, unsigned severity, const char* fmt,
		va_list args);
