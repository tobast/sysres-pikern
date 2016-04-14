#include "sleep.h"
#include "common.h"

s32 volatile * const TIMER = (s32 volatile*)0x20003000;

void sleep_us(int us) {
	/** Sleeps (busy sleep) for [us] microseconds.
	 * NOTE: May not work if us is too close to 0 or max_Int (32b) */
	u32 init_count = TIMER[1];
	while ((u32)(TIMER[1] - init_count) < (u32) us) {};
}

