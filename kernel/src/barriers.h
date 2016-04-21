/****
 * Cache invalidation and barriers functions, see
 * https://github.com/raspberrypi/firmware/wiki/Accessing-mailboxes
 ****/

#pragma once

/// The following functions are taken from
/// https://github.com/raspberry-alpha-omega/corn-mainline/

/*
 * Clean and invalidate entire cache
 * Flush pending writes to main memory
 * Remove all data in data cache
 */
inline void flushcache() {
	asm volatile ("mcr p15, #0, %[zero], c7, c14, #0" : : [zero] "r" (0) );
}

/*
 * Data memory barrier
 * No memory access after the DMB can run until all memory accesses before it
 * have completed
 */
inline void dataMemoryBarrier() {
	asm volatile ("mcr p15, #0, %[zero], c7, c10, #5" : : [zero] "r" (0) );
}

/*
 * Data synchronisation barrier
 * No instruction after the DSB can run until all instructions before it have
 * completed
 */
inline void dataSyncBarrier() {
	asm volatile ("mcr p15, #0, %[zero], c7, c10, #4" : : [zero] "r" (0) );
}

