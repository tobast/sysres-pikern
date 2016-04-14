#include "assert.h"
#include "gpio.h"

void assert(bool b) {
	if (!b) {
		crash();
	}
}

void crash() {
	gpioSet(ACT_GPIO);
	gpioSet(LED_GPIO);
	while (1) {}
}
