#include "gpio.h"

namespace gpio {
	void init() {
		gpio::setWay(gpio::ACT_PIN, gpio::WAY_OUTPUT);
		gpio::setWay(gpio::CRASH_PIN, gpio::WAY_OUTPUT);
		gpio::setWay(gpio::LED_PIN, gpio::WAY_OUTPUT);
		for(int i=0; i < 8; i++) {
			setWay(BYTE_PINS[i], gpio::WAY_OUTPUT);
			unset(BYTE_PINS[i]);
		}

		gpio::set(gpio::ACT_PIN);
		gpio::unset(gpio::CRASH_PIN);
		gpio::unset(gpio::LED_PIN);
	}
}
