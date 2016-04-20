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

	sleep_us(1000); // Let everything settle down
	// (If we don't, mailbox will not work)

	uint64_t mac = mailbox::getMac();
	gpio::blinkValue((uint32_t) mac);
	gpio::blinkValue((uint32_t)(mac >> 32));

	uint32_t model = mailbox::getBoardModel();
	gpio::blinkValue(model);
	uint32_t rev = mailbox::getBoardRevision();
	gpio::blinkValue(rev);
	uint32_t ramSize = mailbox::getRamSize();
	gpio::blinkValue(ramSize);

	crash();

	async_start(&act_blink, NULL);
	async_start(&led_blink, NULL);
	enable_irq();
	async_go();
}

