/***
 * Contains every hardware-dependant constants.
 ***/

#pragma once

#include "common.h"

namespace hardware {
	namespace mailbox {
		// Mailbox protocol
		u32 volatile* const READ = (u32*) 0x2000b880;
		u32 volatile* const STATUS = (u32*) 0x2000b898;
		u32 volatile* const WRITE = (u32*) 0x2000b880;
	}

#ifdef HW_PI_1B
	s32 volatile * const GPIO = (s32 volatile *)0x20200000;

	s32 volatile* const IRQ = (s32 volatile*) 0x2000B200;
	s32 volatile* const ARM_TIMER = (s32 volatile *)0x2000B400;

	s32 volatile * const GPIO_TIMER = (s32 volatile*)0x20003000;

#endif//HW_PI_1B
}

