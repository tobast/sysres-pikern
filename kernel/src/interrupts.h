#pragma once

#include "common.h"

typedef void (*handler)(void*);
typedef void (*handler_svc)(void*, int);

inline void enable_irq() {
	asm(
		"mrs r0,cpsr\n\t"
		"bic r0,r0,#0x80\n\t"
		"msr cpsr_c,r0\n\t"
	: : : "r0");
}

s32 get_cpsr();

inline void disable_irq() {
	asm(
		"mrs r0,cpsr\n\t"
		"orr r0,r0,#0x80\n\t"
		"msr cpsr_c,r0\n\t"
	: : : "r0");
}

inline void enable_fiq() {
	asm(
		"mrs r0,cpsr\n\t"
		"bic r0,r0,#0x40\n\t"
		"msr cpsr_c,r0\n\t"
	: : : "r0");
}

inline void disable_fiq() {
	asm(
		"mrs r0,cpsr\n\t"
		"orr r0,r0,#0x40\n\t"
		"msr cpsr_c,r0\n\t"
	: : : "r0");
}

void set_irq_handler(handler irq_handler);
void set_svc_handler(handler_svc svc_handler);
void init_vector_table();
void init_stacks();
