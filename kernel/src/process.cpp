#include "assert.h"
#include "barriers.h"
#include "hardware_constants.h"
#include "interrupts.h"
#include "pool_allocator.h"
#include "process.h"
#include "sleep.h"
#include "svc.h"
#include "uspi_interface.h"

#include "gpio.h" // FIXME DEBUG

struct context {
	s32 spsr;
	s32 lr;
	s32 r0;
	s32 r1;
	s32 r2;
	s32 r3;
	s32 r4;
	s32 r5;
	s32 r6;
	s32 r7;
	s32 r8;
	s32 r9;
	s32 r10;
	s32 r11;
	s32 r12;
	s32 r13;
	s32 r14;
	context () {
		r0 = r1 = r2 = r3 = r4 = r5 = r6 = r7 = 0;
		r8 = r9 = r10 = r11 = r12 = r13 = r14 = 0;
		spsr = lr = 0;
	};
};

enum pstate {
	PROCESS_ACTIVE,
	PROCESS_WAIT_READ,
	PROCESS_WAIT_WRITE
};

struct process {
	context cont;
	int next_process;
	int previous_process;
	pstate process_state;
	int state_info;
};

const int NUMBER_INTERRUPTS = 64;
struct interrupt_context {
	TInterruptHandler* handler;
	void* param;
};
interrupt_context bound_interrupts[NUMBER_INTERRUPTS];
void init_interrupts() {
	for (int i = 0; i < NUMBER_INTERRUPTS; i++) {
		bound_interrupts[i].handler = NULL;
	}
}
void ConnectInterrupt(unsigned nIRQ, TInterruptHandler *pHandler, void *pParam) {
	bound_interrupts[nIRQ].handler = pHandler;
	bound_interrupts[nIRQ].param = pParam;
	hardware::IRQ_ENABLE_1[nIRQ >> 5] = 1 << (nIRQ & 31);
}


const int MAX_TIMERS = 256;
struct timer_context {
	TKernelTimerHandler* handler;
	void* param;
	void* context;
	u64 trigger_time;
};

timer_context timers[MAX_TIMERS];
u64 first_trigger = (u64)(-1);
void init_timers() {
	for (int i = 0; i < MAX_TIMERS; i++) {
		timers[i].handler = NULL;
	}
}
int get_timer_context() {
	for (int i = 0; i < MAX_TIMERS; i++) {
		if (timers[i].handler == NULL) {
			return i;
		}
	}
	assert(false, 0x13);
	return 0;
}
void recalc_trigger() {
	first_trigger = (u64)(-1);
	for (int i = 0; i < MAX_TIMERS; i++) {
		if (timers[i].handler != NULL) {
			first_trigger = min(first_trigger, timers[i].trigger_time);
		}
	}
}
void check_and_run_timers() {
	u64 current_time = elapsed_us();
	if (current_time < first_trigger) return;
	for (int i = 0; i < MAX_TIMERS; i++) {
		if (timers[i].handler != NULL && timers[i].trigger_time <= current_time) {
			timers[i].handler(i, timers[i].param, timers[i].context);
			timers[i].handler = NULL;
		}
	}
	recalc_trigger();
}
unsigned StartKernelTimer (unsigned nHzDelay,
		TKernelTimerHandler *pHandler, void *pParam, void *pContext) {
	u64 current_time = elapsed_us();
	int i = get_timer_context();
	timers[i].handler = pHandler;
	timers[i].param = pParam;
	timers[i].context = pContext;
	timers[i].trigger_time = current_time + nHzDelay * 1000000 / HZ;
	return (unsigned)i;
}
void CancelKernelTimer (unsigned timer) {
	timers[timer].handler = NULL;
}


const int MAX_PROCESS = 10;
// TODO: dynamic allocation?
process processes[MAX_PROCESS];
int active_process = 0;
int free_process = 1;

const int SOCKET_BUFFER_SIZE = 1024;
const int MAX_SOCKETS = 10;
// TODO: file/socket descriptor table
struct socket {
	int start;
	int length;
	char buffer[SOCKET_BUFFER_SIZE];
};
socket sockets[MAX_SOCKETS];
pool_allocator<MAX_SOCKETS, socket, sockets> sockets_allocator;
//int free_socket = 0;
//socket sockets[MAX_SOCKETS];

int create_socket() {
	int i = sockets_allocator.alloc();
	//int i = free_socket;
	//assert (i < MAX_SOCKETS);
	//free_socket = sockets[i].start;
	sockets[i].start = 0;
	sockets[i].length = 0;
	return i;
}

void close_socket(int i) {
	// TODO: processes that use that socket
	// --> file descriptors table?
	sockets_allocator.dealloc(i);
	//sockets[i].start = free_socket;
	//free_socket = i;
}

int sread(int i, void* addr, int num) {
	int do_read = min(sockets[i].length, num);
	int index = sockets[i].start;
	for (int k = 0; k < do_read; k++) {
		((char*)addr)[k] = sockets[i].buffer[index];
		index++;
		if (index == SOCKET_BUFFER_SIZE) {
			index = 0;
		}
	}
	sockets[i].start = index;
	sockets[i].length -= do_read;
	return do_read;
}

int swrite(int i, void* addr, int num) {
	int do_write = min(SOCKET_BUFFER_SIZE - sockets[i].length, num);
	int index = (sockets[i].start + sockets[i].length) % SOCKET_BUFFER_SIZE;
	for (int k = 0; k < do_write; k++) {
		sockets[i].buffer[index] = ((char*)addr)[k]; 
		index++;
		if (index == SOCKET_BUFFER_SIZE) {
			index = 0;
		}
	}
	sockets[i].length += do_write;
	return do_write;
}

void reset_timer() {
	hardware::ARM_TIMER[3] = 0;
	hardware::ARM_TIMER[0] = 500;
}

void go_next_process(context* current_context) {
	processes[active_process].cont = *current_context;
	next_process();
	*current_context = processes[active_process].cont;
}

extern "C" void on_irq(void* stack_pointer) {
	dataMemoryBarrier();
	for (int i = 0; i < NUMBER_INTERRUPTS; i++) {
		if (hardware::IRQ_PENDING_1[i >> 5] & (1 << (i & 31))) {
			if (bound_interrupts[i].handler != NULL) {
				bound_interrupts[i].handler(bound_interrupts[i].param);
				dataMemoryBarrier();
			} else {
				hardware::IRQ_DISABLE_1[i >> 5] = 1 << (i & 31);
			}
		}
	}
	dataMemoryBarrier();
	if ((*hardware::IRQ_BASIC_PENDING) & 1) {
		check_and_run_timers();
		go_next_process((context*)stack_pointer);
		reset_timer();
	}
	dataMemoryBarrier();
}

extern "C" void on_svc(void* stack_pointer, int svc_number) {
	context* current_context = (context*)stack_pointer;
	switch (svc_number) {
		case SVC_GET_PID: {
			current_context->r0 = active_process;
			return;
		}
		case SVC_READ: {
			int socket = (int)current_context->r0;
			void* addr = (void*)current_context->r1;
			int size = (int)current_context->r2;
			int num_read = sread(socket, addr, size);
			if (num_read > 0) {
				current_context->r0 = num_read;
			} else {
				processes[active_process].process_state = PROCESS_WAIT_READ;
				go_next_process(current_context);
				reset_timer();
			}
			return;
		}
		case SVC_WRITE: {
			int socket = (int)current_context->r0;
			void* addr = (void*)current_context->r1;
			int size = (int)current_context->r2;
			int num_written = swrite(socket, addr, size);
			if (num_written > 0) {
				current_context->r0 = num_written;
			} else {
				processes[active_process].process_state = PROCESS_WAIT_WRITE;
				go_next_process(current_context);
				reset_timer();
			}
			return;
		}
		case SVC_SLEEP: {
			// TODO
			return;
		}
		default:
			// Invalid svc
			// Kill process?
			return;
	}
}

void init_process_table() {
	processes[0].next_process = 0;
	processes[0].previous_process = 0;
	processes[0].process_state = PROCESS_ACTIVE;
	active_process = 0;
	free_process = 1;
	for (int i = 1; i < MAX_PROCESS; i++) {
		processes[i].next_process = i + 1;
	}
	
	//free_socket = 0;
	//for (int i = 0; i < MAX_SOCKETS; i++) {
	//	sockets[i].start = i + 1;
	//}
	sockets_allocator.init();

	set_irq_handler(&on_irq);
	set_svc_handler(&on_svc);

	init_timers();

	*hardware::IRQ_ENABLE_BASIC |= 1;
	hardware::ARM_TIMER[3] = 0;
	hardware::ARM_TIMER[7] = 0xff;
	hardware::ARM_TIMER[2] = 0x3e00a2;
	hardware::ARM_TIMER[0] = 500;
	hardware::ARM_TIMER[6] = 500;
}

// Add a process before active_process
void add_process(int i) {
	int u = active_process;
	int v = processes[u].previous_process;
	processes[u].previous_process = i;
	processes[v].next_process = i;
	processes[i].previous_process = v;
	processes[i].next_process = u;
	processes[i].process_state = PROCESS_ACTIVE;
	// There was no active process before
	if (active_process == 0) {
		active_process = i;
	}
}

void delete_process(int i) {
	int u = processes[i].previous_process;
	int v = processes[i].next_process;
	processes[u].next_process = v;
	processes[v].next_process = u;
	processes[i].next_process = free_process;
	free_process = i;
	// TODO
	if (active_process == i) {
		active_process = u;
		if (active_process == 0) {
			active_process = processes[u].previous_process;
		}
	}
}

int create_process() {
	int i = free_process;
	assert(i < MAX_PROCESS);
	free_process = processes[i].next_process;
	add_process(i);
	return i;
}

bool ready(int i) {
	switch (processes[i].process_state) {
		case PROCESS_ACTIVE: {
			return true;
		}
		case PROCESS_WAIT_READ: {
			int sock = processes[i].cont.r0;
			if (sockets[sock].length > 0) {
				// TODO: quand y'aura le MMU, danger...
				int num_read = sread(sock, (void*)processes[i].cont.r1,
				                                  processes[i].cont.r2);
			    assert(num_read > 0);
				processes[i].cont.r0 = num_read;
				processes[i].process_state = PROCESS_ACTIVE;
				return true;
			} else {
				return false;
			}
		}
		case PROCESS_WAIT_WRITE: {
			int sock = processes[i].cont.r0;
			if (sockets[sock].length < SOCKET_BUFFER_SIZE) {
				// TODO: quand y'aura le MMU, danger...
				int num_written = swrite(sock, (void*)processes[i].cont.r1,
				                                      processes[i].cont.r2);
			    assert(num_written > 0);
				processes[i].cont.r0 = num_written;
				processes[i].process_state = PROCESS_ACTIVE;
				return true;
			} else {
				return false;
			}
		}
		default: return false;

	}
}

void next_process() {
	do {
		active_process = processes[active_process].next_process;
	} while (!ready(active_process));
	if (active_process == 0) {
		do {
			active_process = processes[active_process].next_process;
		} while (!ready(active_process));
	}
}

void async_start(async_func f, void* arg, s32 mode) {
	int i = create_process();
	processes[i].cont = context();
	processes[i].cont.lr = ((s32)f) + 4;
	processes[i].cont.r0 = (s32)arg;
	// Find a suitable stack pointer; TODO: make that better
	processes[i].cont.r13 = 0x1000000 + 0x100000 * (i + 1);
	processes[i].cont.spsr = mode;
}

__attribute__((naked))
void _async_go(context*) {
	asm(
		"mov sp, r0\n\t"
		"ldmfd sp!, {r0, lr}\n\t"
		"msr spsr_cxsf, r0\n\t"
		"ldmfd sp, {r0-lr}^\n\t"
		"add sp, sp, #60\n\t"
		"nop\n\t" // Needed?
		"subs pc, lr, #4"
	);
}
void wait() {
	while (1) {
		asm volatile ("wfi");
	}
}
void async_go() {
	// Init the waiting process
	processes[0].cont = context();
	processes[0].cont.lr = ((s32)&wait) + 4;
	// Find a suitable stack pointer; TODO: make that better
	processes[0].cont.r13 = 0x1100000;
	s32 cpsr = get_cpsr();
	cpsr &= ~0xdf;
	cpsr |= 0x53;
	processes[0].cont.spsr = 0x53; // SVC mode, enable IRQ, disable FIQ
	context* svc_stack_address = (context*)(0x6000);
	*svc_stack_address = processes[active_process].cont;
	_async_go(svc_stack_address);
}
