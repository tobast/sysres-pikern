#include "gpio.h"

namespace gpio {
	void init() {
		gpio::setWay(gpio::ACT_PIN, gpio::WAY_OUTPUT);
		gpio::setWay(gpio::CRASH_PIN, gpio::WAY_OUTPUT);

		gpio::set(gpio::ACT_PIN);
		gpio::unset(gpio::CRASH_PIN);
	}
}
