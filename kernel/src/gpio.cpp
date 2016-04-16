#include "gpio.h"

namespace gpio {
	void init() {
		gpio::setWay(gpio::ACT_PIN, gpio::WAY_OUTPUT);
		gpio::setWay(gpio::CRASH_PIN, gpio::WAY_OUTPUT);

		gpio::set(gpio::ACT_PIN);
		gpio::unset(gpio::CRASH_PIN);
	}

	void blinkValue(uint32_t val) {
		static const int TIME_STEP = 500*1000; // us
		
		// Firing 4 short edges, to tell the reader the message will start
		for(int i=0; i < 4; i++) {
			set(LED_PIN);
			sleep_us(TIME_STEP/10);
			unset(LED_PIN);
			sleep_us(TIME_STEP/10);
		}

		sleep_us(TIME_STEP);
		while(val > 0) {
			set(LED_PIN);
			setTo(LED2_PIN, val%2 > 0);
			val /= 2;
			sleep_us(TIME_STEP/10);
			unset(LED_PIN);
			sleep_us(TIME_STEP);
		}

		unset(LED_PIN);
		unset(LED2_PIN);
	}
}
