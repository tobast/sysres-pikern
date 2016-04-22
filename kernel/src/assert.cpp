#include "assert.h"
#include "gpio.h"

void assert(bool b, uint8_t assertId) {
	if (!b) {
		gpio::dispByte(assertId);
		crash();
	}
}

void crash() {
	gpio::set(gpio::ACT_PIN);
	gpio::unset(gpio::LED_PIN);
	gpio::set(gpio::CRASH_PIN);
	while (1) {}
}
