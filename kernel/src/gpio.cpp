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

	void blink(int pin) {
		unset(pin);
		sleep_us(100*1000);
		set(pin);
		sleep_us(200*1000);
		unset(pin);
		sleep_us(100*1000);
	}

	void dispByte(uint8_t val) {
		for(int pos=0; pos < 8; pos++)
			setVal(BYTE_PINS[pos], val & (0x1 << pos));
	}

	void blinkValue(uint32_t val) {
		for(int i=0; i < 4; i++) {
			set(LED_PIN);
			dispByte((val >> (8*i)) & 0xFF);
			sleep_us(200*1000);
			unset(LED_PIN);
			sleep_us(1800*1000);
		}
	}

}

