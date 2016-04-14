#pragma once

#include "common.h"

s32 volatile * const GPIO = (s32 volatile *)0x20200000;

const int LED_GPIO = 25;
const int ACT_GPIO = 16;

const char GPIO_WAY_INPUT  = 0b000;
const char GPIO_WAY_OUTPUT = 0b001;

inline void gpioSet(int i) {
	// Sets the bit i of GPIO + 0x1C
	GPIO[7 + (i >> 5)] = 1 << (i & 0x1f);
}

inline void gpioUnset(int i) {
	// Sets the bit i of GPIO + 0x28
	GPIO[10 + (i >> 5)] = 1 << (i & 0x1f);
}

inline void gpioSetWay(int i, int way) {
	// Sets the 3 bits at position 3 * (i % 10) of
	// (GPIO + 4 * i // 10) to way
	GPIO[i / 10] = way << (3 * (i % 10));
}
