#pragma once

#include "common.h"
#include "hardware_constants.h"
#include "sleep.h"

namespace gpio {
	const int LED_PIN = 25;
	const int ACT_PIN = 16;
	const int CRASH_PIN = 7;

	const int BYTE_PINS[] = { 2,3,4, 9,10,11, 14,15 };

	const char WAY_INPUT  = 0x0; //0b000
	const char WAY_OUTPUT = 0x1; //0b001

	void init();

	inline void set(int i) {
		// Sets the bit i of GPIO + 0x1C
		hardware::GPIO[7 + (i >> 5)] = 1 << (i & 0x1f);
	}

	inline void unset(int i) {
		// Sets the bit i of GPIO + 0x28
		hardware::GPIO[10 + (i >> 5)] = 1 << (i & 0x1f);
	}

	inline void setVal(int i, bool v) {
		if(v)
			set(i);
		else
			unset(i);
	}

	inline void setWay(int i, int way) {
		// Sets the 3 bits at position 3 * (i % 10) of
		// (GPIO + 4 * i // 10) to way
		int shift = 3 * (i % 10);
		s32 masked = hardware::GPIO[i / 10] & (~(7 << shift));
		hardware::GPIO[i / 10] = masked | (way << shift);
	}

	void dispByte(uint8_t val);
	void blinkValue(uint32_t val);
}
