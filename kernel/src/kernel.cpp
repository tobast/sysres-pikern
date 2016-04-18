#include "assert.h"
#include "common.h"
#include "gpio.h"
#include "interrupts.h"
#include "mailbox.h"
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

#include "barriers.h" // FIXME debug
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

	volatile uint32_t* req_buffer = (volatile uint32_t*)0x10F00000;
   	req_buffer[0]=32;
	req_buffer[1]=0;
	req_buffer[2]=0x00010003;
	req_buffer[3]=8;
	req_buffer[4]=0;
	req_buffer[5]=0;
	req_buffer[6]=0;
	req_buffer[7]=0;

	flushcache();
	assert(hardware::mailbox::STATUS[0] & 0x40000000);

	dataMemoryBarrier();
	hardware::mailbox::WRITE[0] = (uint32_t)req_buffer | 0x08;

	for(int wait=0; !(hardware::mailbox::STATUS[0] & 0x80000000); wait++) {
		flushcache();
		if(wait > (1<<20))
			break;
	}

	dataMemoryBarrier();
	uint32_t data = hardware::mailbox::READ[0];
	dataMemoryBarrier();
	assert((data & 0xF) == 8);
	uint32_t* dataPtr = (uint32_t*)(data & 0xFFFFFFF0);

	gpio::blinkValue(dataPtr[5]);
	gpio::blinkValue(dataPtr[6]);

	sleep_us(1000*1000);
	crash();

	async_start(&act_blink, NULL);
	async_start(&led_blink, NULL);
	enable_irq();
	async_go();
}

