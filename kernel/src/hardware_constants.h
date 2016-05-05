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
		u32 volatile* const WRITE = (u32*) 0x2000b8a0;
	}

#ifdef HW_PI_1B
	s32 volatile * const GPIO = (s32 volatile *)0x20200000;

	s32 volatile* const IRQ_BASE = (s32 volatile*) 0x2000B200;
	s32 volatile* const IRQ_BASIC_PENDING = (s32 volatile*) 0x2000B200;
	s32 volatile* const IRQ_PENDING_1 = (s32 volatile*) 0x2000B204;
	s32 volatile* const IRQ_PENDING_2 = (s32 volatile*) 0x2000B208;
	s32 volatile* const IRQ_ENABLE_1 = (s32 volatile*) 0x2000B210;
	s32 volatile* const IRQ_ENABLE_2 = (s32 volatile*) 0x2000B214;
	s32 volatile* const IRQ_ENABLE_BASIC = (s32 volatile*) 0x2000B218;
	s32 volatile* const IRQ_DISABLE_1 = (s32 volatile*) 0x2000B21C;
	s32 volatile* const IRQ_DISABLE_2 = (s32 volatile*) 0x2000B220;
	s32 volatile* const IRQ_DISABLE_BASIC = (s32 volatile*) 0x2000B224;
	
	s32 volatile* const ARM_TIMER = (s32 volatile *)0x2000B400;

	s32 volatile * const GPIO_TIMER = (s32 volatile*)0x20003000;

#endif//HW_PI_1B
}

