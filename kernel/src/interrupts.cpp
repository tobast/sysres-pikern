#include "interrupts.h"

extern handler volatile interrupt_handler;
extern handler volatile asm_irq_handler;
extern s32 _vector_table;
s32 volatile * const vector_table = &_vector_table;
s32 volatile * const vector_table_dst = 0;
const int vector_table_length = 10;
handler volatile irq_handler;

/***************************************
 *************** WARNING ***************
 ***************************************
 * If you modify this, you MUST modify *
 * vector_table_length above to match  *
 * the actual length of the table.     *
 ****************************************/

asm(
".text\n\t"
"_vector_table:\n\t"
	"ldr pc, interrupt_handler\n\t" // reset
	"ldr pc, interrupt_handler\n\t" // undefined
	"ldr pc, interrupt_handler\n\t" // svc
	"ldr pc, interrupt_handler\n\t" // prefetch
	"ldr pc, interrupt_handler\n\t" // abort
	"ldr pc, interrupt_handler\n\t" // reserved
	"ldr pc, asm_irq_handler\n\t" // irq
	"ldr pc, interrupt_handler\n\t" // fiq
"interrupt_handler:\n\t"
	".space 4\n\t"
"asm_irq_handler:\n\t"
    ".space 4\n\t"
);

__attribute__((naked))
void _asm_irq_handler(void*) {
	asm(
		"stmfd sp, {r0-lr}^\n\t" // Save registers
		"sub sp, sp, #60\n\t" // We saved 15 registers
		"mrs r0, spsr\n\t" // Get SPSR
		"stmfd sp!, {r0, lr}\n\t" // Save it and lr as well
		"ldr r1, =irq_handler\n\t"
		"ldr r1, [r1]\n\t"
		"mov r0, sp\n\t" // Save stack pointer as first argument
		"blx r1\n\t"
		"ldmfd sp!, {r0, lr}\n\t" // Restore link register and value of spsr
		"msr spsr_cxsf, r0\n\t"
		"ldmfd sp, {r0-lr}^\n\t" // Restore them
		"add sp, sp, #60\n\t"
		"nop\n\t" // Needed?
		"subs pc, lr, #4" // And... jump.
	);
}

void set_irq_handler(handler int_handler) {
	irq_handler = int_handler;
}

void init_vector_table() {
	for (int i = 0; i < vector_table_length; i++) {
		vector_table_dst[i] = vector_table[i];
	}
	*((handler*)
		((s32)vector_table_dst +
			(s32)(&asm_irq_handler) -
			(s32)vector_table)) = _asm_irq_handler;
}

void init_stacks() {
	asm(
		"mov r0,#0xD2\n\t" // Mode IRQ ; disable IRQ and FIQ
		"msr cpsr_c,r0\n\t"
		"mov sp,#0x8000\n\t"

		"mov r0,#0xD1\n\t" // Mode FIQ ; disable IRQ and FIQ
		"msr cpsr_c,r0\n\t"
		"mov sp,#0x4000\n\t"

		"mov r0,#0xD3\n\t" // Mode SVC ; disable IRQ and FIQ
		"msr cpsr_c,r0\n\t"
   );
}
