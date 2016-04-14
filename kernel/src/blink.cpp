
#include "common.h"
#include "gpio.h"
#include "interrupts.h"
#include "process.h"
#include "assert.h"

#define TIMER ((s32 volatile *)0x20003000)

void sleep_us(int us) {
	/** Sleeps (busy sleep) for [us] microseconds.
	 * NOTE: May not work if us is too close to 0 or max_Int (32b) */
	u32 init_count = TIMER[1];
	while ((u32)(TIMER[1] - init_count) < (u32) us) {};
}


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
	gpio::setWay(gpio::LED_PIN, gpio::WAY_OUTPUT);
	gpio::setWay(gpio::ACT_PIN, gpio::WAY_OUTPUT);
	gpio::setWay(gpio::CRASH_PIN, gpio::WAY_OUTPUT);

	gpio::set(gpio::LED_PIN);
	gpio::unset(gpio::ACT_PIN);
	gpio::unset(gpio::CRASH_PIN);

	async_start(&led_blink, NULL);
	async_start(&act_blink, NULL);
	enable_irq();
	async_go();
}

