#pragma once

#include <cstdarg>

#include "Bytes.h"
#include "gpio.h"

void formatToBytes(Bytes& dest, const char* fmt, va_list args);

