#include "sleep.h"
//#include "common.h" // Now included in header
#include "hardware_constants.h"

void sleep_us(int us) {
	/** Sleeps (busy sleep) for [us] microseconds.
	 * NOTE: May not work if us is too close to 0 or max_Int (32b) */
	u32 init_count = hardware::GPIO_TIMER[1];
	while ((u32)(hardware::GPIO_TIMER[1] - init_count) < (u32) us) {};
}

u32 ellapsed_us() {
	return hardware::GPIO_TIMER[1];
	// Copied from above. @Ekdohibs: TODO review
}

