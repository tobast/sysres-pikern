#include "process.h"
#include "assert.h"
#include "interrupts.h"

s32 volatile* const IRQ = (s32 volatile*) 0x2000B200;
s32 volatile* const ARM_TIMER = (s32 volatile *)0x2000B400;

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
struct process {
	context cont;
	int next_process;
	int previous_process;
};

const int MAX_PROCESS = 10;
process processes[MAX_PROCESS];
int active_process = 0;
int free_process = 1;


extern "C" void on_irq(void* stack_pointer) {
	context* current_context = (context*)stack_pointer;
	processes[active_process].cont = *current_context;
	next_process();
	*current_context = processes[active_process].cont;
	ARM_TIMER[3] = 0;
	ARM_TIMER[0] = 500;
}

void init_process_table() {
	processes[0].next_process = 0;
	processes[0].previous_process = 0;
	active_process = 0;
	free_process = 1;
	for (int i = 1; i < MAX_PROCESS; i++) {
		processes[i].next_process = i + 1;
	}

	set_irq_handler(&on_irq);
	IRQ[6] |= 1;
	ARM_TIMER[3] = 0;
	ARM_TIMER[7] = 0xff;
	ARM_TIMER[2] = 0x3e00a2;
	ARM_TIMER[0] = 500;
	ARM_TIMER[6] = 500;
}

// Add a process before active_process
void add_process(int i) {
	int u = active_process;
	int v = processes[u].previous_process;
	processes[u].previous_process = i;
	processes[v].next_process = i;
	processes[i].previous_process = v;
	processes[i].next_process = u;
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

void next_process() {
	active_process = processes[active_process].next_process;
	if (active_process == 0) {
		active_process = processes[active_process].next_process;
	}
}

void async_start(async_func f, void* arg) {
	int i = create_process();
	processes[i].cont = context();
	processes[i].cont.lr = ((s32)f) + 4;
	processes[i].cont.r0 = (s32)arg;
	// Find a suitable stack pointer; TODO: make that better
	processes[i].cont.r13 = 0x1000000 + 0x100000 * (i + 1);
	processes[i].cont.spsr = 0x50; // User mode, enable IRQ, disable FIQ
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
void async_go() {
	assert(active_process != 0);
	_async_go(&(processes[active_process].cont));
}

