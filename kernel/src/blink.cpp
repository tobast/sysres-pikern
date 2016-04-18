
#include "common.h"
#include "gpio.h"
#include "interrupts.h"
#include "process.h"
#include "assert.h"
#include "svc.h"

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

extern "C" void led_blink_writer(void* arg) {
	int sock = (int)arg;
	char zero = 0;
	char one = 1;
	while(1) {
		write(sock, (void*)&one, 1);
		sleep_us(2*500000);
		write(sock, (void*)&zero, 0);
		sleep_us(500000);
	}
}

extern "C" void led_blink_listener(void* arg) {
	int sock = (int)arg;
	while(1) {
		char c;
		read(sock, (void*)&c, 1);
		if (c) {
			gpio::set(gpio::LED_PIN);
		} else {
			gpio::unset(gpio::LED_PIN);
		}
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
	// Actually, don't: it doesn't work
	// TODO: see if it is possible to make it work
	// Switch to system mode
	// We can't do that later since it will corrupt stacks
	//asm ("mov r0,#0xDF\n\t" // System mode ; disable IRQ and FIQ
	//	 "msr cpsr_c,r0\n\t"
	//	 "mov sp,#0x6000\n\t" // Init stack pointer
	//	 : : : "r0");
	init_vector_table();
	init_stacks();
	init_process_table();

	gpio::init();
	gpio::setWay(gpio::LED_PIN, gpio::WAY_OUTPUT);
	gpio::unset(gpio::LED_PIN);

	async_start(&act_blink, NULL);
	//async_start(&led_blink, NULL);
	
	int socket = create_socket();
	async_start(&led_blink_writer, (void*)socket);
	async_start(&led_blink_listener, (void*)socket);
	enable_irq();
	async_go();
}

