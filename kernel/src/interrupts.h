#pragma once

#include "common.h"

typedef void (*handler)(void*);

inline void enable_irq() {
	asm(
		"mrs r0,cpsr\n\t"
		"bic r0,r0,#0x80\n\t"
		"msr cpsr_c,r0\n\t"
	: : : "r0");
}

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

void set_irq_handler(handler int_handler);
void init_vector_table();
void init_stacks();
