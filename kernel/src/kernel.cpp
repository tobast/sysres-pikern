#include "assert.h"
#include "common.h"
#include "gpio.h"
#include "interrupts.h"
#include "process.h"
#include "sleep.h"

extern "C" void led_blink(void*) {
	while(1) {
		gpio::set(gpio::LED_PIN);
		sleep_us(2*500000);
		gpio::unset(gpio::LED_PIN);
		sleep_us(500000);
	}
}

extern "C" void act_blink(void*) {
	while(1) {
		gpio::set(gpio::ACT_PIN);
		sleep_us(2*300000);
		gpio::unset(gpio::ACT_PIN);
		sleep_us(300000);
	}
}

__attribute__((naked))
__attribute__((section(".init")))
int main(void) {
	init_vector_table();
	init_stacks();
	init_process_table();

	gpio::init();
	gpio::setWay(gpio::LED_PIN, gpio::WAY_OUTPUT);
	gpio::unset(gpio::LED_PIN);

	async_start(&led_blink, NULL);
	async_start(&act_blink, NULL);
	enable_irq();
	async_go();
}

