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
#include "logger.h"
#include "filesystem.h"
#include "exec_context.h"

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
	logger::init();

//	logger::addListener(0x81c79d16); // tobast-laptop

	gpio::blink(gpio::LED_PIN);
	
	appendLog(
#include "start_message.h"
			);

	assert(USPiInitialize() != 0, 0xFF);
	appendLog(LogInfo, "USPi", "Initialization took %u μs",
			(uint32_t)elapsed_us());
	sleep(2 * 1000 * 1000);
	gpio::blink(gpio::LED_PIN);
	gpio::blink(gpio::LED_PIN);

	assert(USPiEthernetAvailable() != 0, 0x01);

	async_start(((void(*)(void*))&nw::packetHandlerStart), NULL, 0x5f,
			"pck_handler");
	async_start(((void(*)(void*))&logger::mainLoop), NULL, 0x50,
			"syslogger");

	/*
	while(true) {
		appendLog(LogDebug, "dbg", "Hello, world!");
		sleep(1000*1000);
	}*/

	async_start(&led_blink, NULL, 0x50, "led_blink");

//	int socket = create_socket();
//	async_start(&byte_blink_writer, (void*)socket);
//	async_start(&byte_blink_listener, (void*)socket);

	execution_context *ec = (execution_context*)
		(malloc(sizeof(execution_context)));
	int stdin_socket = create_socket(nw::bindUdpPort(4042));
	int stdout_socket = create_socket();
	ec->stdin = stdin_socket;
	ec->stdout = stdout_socket;
	ec->argc = 1;
	ec->argv = (char**)(malloc(2 * sizeof(const char*)));
	const char* filename = "Balances.z5";
	const int len = str_len(filename);
	ec->argv[1] = (char*)(malloc(len));
	for (int i = 0; i < len; i++) {
		ec->argv[1][i] = filename[i];
	}
	node *file = follow_path("bin/zmachine");
	assert(file != NULL, 0x99);
	int u = run_process(file, ec);
	assert(u != -1, 0x9a);
	// TODO: recognize EOF
	while (true) {
		char buffer[257];
		if (is_ready_read(stdout_socket)) {
			int n = read(stdout_socket, (void*)buffer, 256);
			buffer[n] = '\0';
//			appendLog(LogDebug, "main", buffer);
			UdpSysData outPacket(0x0a000001, 42, 4042, buffer, n);
			udp_write(&outPacket);
		} else if (!is_process_alive(u)) {
			break;
		} else {
			sleep(200);
	    }
	}
	wait(u);
	appendLog(LogDebug, "dbg", "Process finished!");

	exit(0);
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

	mallocInit();

	init_vector_table();
	init_stacks();
	init_process_table();
	init_filesystem();

	gpio::init();
	gpio::setWay(gpio::LED_PIN, gpio::WAY_OUTPUT);
	gpio::unset(gpio::LED_PIN);

	enable_irq();
	async_start(&kernel_run, NULL, 0x5f,
			"kernel"); // System mode, enable IRQ, disable FIQ
	async_start(&act_blink, NULL, 0x50, "act_blink");

	async_go();
}

