#include "assert.h"
#include "common.h"
#include "gpio.h"
#include "interrupts.h"
#include "mailbox.h"
#include "malloc.h"
#include "process.h"
#include "sleep.h"
#include "svc.h"
#include "networkCore.h"

#include <uspi.h>

extern "C" void led_blink(void*) {
	while(1) {
		asm volatile ("svc #42");
		gpio::set(gpio::LED_PIN);
		sleep(2*500000);
		gpio::unset(gpio::LED_PIN);
		sleep(500000);
	}
}

extern "C" void act_blink(void*) {
	while(1) {
		asm volatile ("svc #43");
		gpio::set(gpio::ACT_PIN);
		sleep(100*1000);
		gpio::unset(gpio::ACT_PIN);
		sleep(100*1000);
	}
}

extern "C" void byte_blink_writer(void* arg) {
	int sock = (int)arg;
	char n = 0;
	while(1) {
		write(sock, (void*)&n, 1);
		sleep(350000);
		n++;
	}
}

extern "C" void byte_blink_listener(void* arg) {
	int sock = (int)arg;
	while(1) {
		char c;
		read(sock, (void*)&c, 1);
		gpio::dispByte(c);
	}
}

uint32_t invEndianness(uint32_t v) {
	uint32_t out=0;
	for(int i=0; i < 4; i++) {
		out <<= 8;
		out += v & 0xff;
		v >>= 8;
	}
	return out;
}

void kernel_run(void*) {
	nw::init();

	sleep(2 * 1000 * 1000);
	gpio::blink(gpio::LED_PIN);
	
	nw::logAppend(
#include "start_message.h"
			);

	assert(USPiInitialize() != 0, 0xFF);
	sleep(2 * 1000 * 1000);
	gpio::blink(gpio::LED_PIN);
	gpio::blink(gpio::LED_PIN);

	assert(USPiEthernetAvailable() != 0, 0x01);

	async_start(((void(*)(void*))&nw::packetHandlerStart), NULL, 0x5f);

	nw::logAppend("Hello, world!\n");

	async_start(&led_blink, NULL);
	async_start(&act_blink, NULL);

	int socket = create_socket();
	async_start(&byte_blink_writer, (void*)socket);
	async_start(&byte_blink_listener, (void*)socket);

	// Fixme: replace that with a clean exit call
	while (1) {
		asm volatile ("wfi");
	}
}

void kernel_main(void) {
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

	mallocInit();

	gpio::init();
	gpio::setWay(gpio::LED_PIN, gpio::WAY_OUTPUT);
	gpio::unset(gpio::LED_PIN);

	enable_irq();
	async_start(&act_blink, NULL);
	async_start(&kernel_run, NULL, 0x5f); // System mode, enable IRQ, disable FIQ

	async_go();
}

