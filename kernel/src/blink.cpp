#include <cstdint>
typedef int32_t Int;
//typedef int Int;
#define GPIO	((Int volatile *)0x20200000)
#define TIMER ((Int volatile *)0x20003000)

#define IRQ ((Int volatile *)0x2000B200)
#define ARM_TIMER ((Int volatile *)0x2000B400)

const char GPIO_WAY_INPUT =		0b000;
const char GPIO_WAY_OUTPUT =	0b001;

const Int MAX_PROCESS = 10;
#define NULL (0)

typedef void (*interrupt_handler)(Int*);
extern interrupt_handler _interrupt_handler_addr;
interrupt_handler volatile * const interrupt_handler_addr = &_interrupt_handler_addr; 
//extern interrupt_handler _irq_handler_addr;
//interrupt_handler volatile * const irq_handler_addr = &_irq_handler_addr;
extern interrupt_handler _asm_irq_handler_addr;
interrupt_handler volatile * const asm_irq_handler_addr = &_asm_irq_handler_addr;
extern Int _vector_table;
Int volatile * const vector_table = &_vector_table;
Int volatile * const vector_table_dst = 0;
const Int vector_table_length = 10;

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
	"ldr pc, _interrupt_handler_addr\n\t" // reset
	"ldr pc, _interrupt_handler_addr\n\t" // undefined
	"ldr pc, _interrupt_handler_addr\n\t" // svc
	"ldr pc, _interrupt_handler_addr\n\t" // prefetch
	"ldr pc, _interrupt_handler_addr\n\t" // abort
	"ldr pc, _interrupt_handler_addr\n\t" // reserved
	//"ldr pc, _interrupt_handler_addr\n\t" // irq
	//"b _asm_irq_handler\n\t" // irq
	"ldr pc, _asm_irq_handler_addr\n\t" // irq
	"ldr pc, _interrupt_handler_addr\n\t" // fiq
"_interrupt_handler_addr:\n\t"
	".space 4\n\t"
"_asm_irq_handler_addr:\n\t"
    ".space 4\n\t"
);

//"_irq_handler_addr:\n\t"
//	".space 4\n\t"

//extern interrupt_handler _irq_handler_addr;
//interrupt_handler volatile * const irq_handler_addr = &_irq_handler_addr;
interrupt_handler _irq_handler_addr;
interrupt_handler volatile * const irq_handler_addr = &_irq_handler_addr;

__attribute__((naked))
void _asm_irq_handler(Int*) {
	asm(
		"stmfd sp, {r0-lr}^\n\t" // Save registers
		"sub sp, sp, #60\n\t" // We saved 15 registers
		"mrs r0, spsr\n\t" // Get SPSR
		"stmfd sp!, {r0, lr}\n\t" // Save it and lr as well
		"ldr r1, =_irq_handler_addr\n\t"
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

void crash();

void assert(bool b) {
	if (!b) {
		crash();
	}
}

struct context {
	Int spsr;
	Int lr;
	Int r0;
	Int r1;
	Int r2;
	Int r3;
	Int r4;
	Int r5;
	Int r6;
	Int r7;
	Int r8;
	Int r9;
	Int r10;
	Int r11;
	Int r12;
	Int r13;
	Int r14;
	context () {
		r0 = r1 = r2 = r3 = r4 = r5 = r6 = r7 = 0;
		r8 = r9 = r10 = r11 = r12 = r13 = r14 = 0;
		spsr = lr = 0;
	};
};
struct process {
	context cont;
	Int next_process;
	Int previous_process;
};
process processes[MAX_PROCESS];
Int active_process = 0;
Int free_process = 1;

void init_process_table() {
	processes[0].next_process = 0;
	processes[0].previous_process = 0;
	active_process = 0;
	free_process = 1;
	for (Int i = 1; i < MAX_PROCESS; i++) {
		processes[i].next_process = i + 1;
	}
}

// Add a process before active_process
void add_process(Int i) {
	Int u = active_process;
	Int v = processes[u].previous_process;
	processes[u].previous_process = i;
	processes[v].next_process = i;
	processes[i].previous_process = v;
	processes[i].next_process = u;
	// There was no active process before
	if (active_process == 0) {
		active_process = i;
	}
}

void delete_process(Int i) {
	Int u = processes[i].previous_process;
	Int v = processes[i].next_process;
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

Int create_process() {
	Int i = free_process;
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

extern "C" void on_irq(Int* stack_pointer) {
	context* current_context = (context*)stack_pointer;
	processes[active_process].cont = *current_context;
	next_process();
	*current_context = processes[active_process].cont;
	ARM_TIMER[3] = 0;
	ARM_TIMER[0] = 500;
}

typedef void (*async_func)(void*);
void async_start(async_func f, void* arg) {
	Int i = create_process();
	processes[i].cont = context();
	processes[i].cont.lr = ((Int)f) + 4;
	processes[i].cont.r0 = (Int)arg;
	// Find a suitable stack pointer; TODO: make that better
	processes[i].cont.r13 = 0x1000000 + 0x800000 * (i + 1);
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

inline void gpioSet(Int i) {
	// Sets the bit i of GPIO + 0x1C
	Int orMask = (1<<(i%32));
	GPIO[7+(i/32)] = orMask;
}
inline void gpioUnset(Int i) {
	// Sets the bit i of GPIO + 0x28
	Int orMask = (1<<(i%32));
	GPIO[10+(i/32)] = orMask;
}
inline void gpioSetWay(Int i, Int way) {
	// Sets the 3 bits at position 3*i%10 of (GPIO+4*i//10) to way
	Int orMask = (way<<(3*(i%10)));
	GPIO[i/10] = orMask;
}

void sleep_us(Int us) {
	/** Sleeps (busy sleep) for [us] microseconds.
	 * NOTE: May not work if us is too close to 0 or max_Int (32b) */
	uint32_t init_count = TIMER[1];
	while ((uint32_t)(TIMER[1] - init_count) < (uint32_t) us) {};
}

void init_vector_table() {
	for (Int i = 0; i < vector_table_length; i++) {
		vector_table_dst[i] = vector_table[i];
	}
	*((interrupt_handler*)((Int)vector_table_dst + (Int)asm_irq_handler_addr - (Int)vector_table)) = &_asm_irq_handler;
}

void set_irq_handler(interrupt_handler handler) {
	*irq_handler_addr = handler;
}

Int volatile count = 0;

const Int LED_GPIO = 25;
const Int ACT_GPIO = 16;

extern "C" void on_irq_blk(Int* stack_pointer) {
	count++;
	if (count & 1) {
		gpioSet(LED_GPIO);
	} else {
		gpioUnset(LED_GPIO);
	}

	ARM_TIMER[3] = 0;
	ARM_TIMER[0] = 500000;
}

extern "C" void led_blink(void*) {
	while(1) {
		gpioSet(LED_GPIO);
		sleep_us(2*500000);
		gpioUnset(LED_GPIO);
		sleep_us(500000);
	}
}

extern "C" void act_blink(void*) {
	while(1) {
		gpioSet(ACT_GPIO);
		sleep_us(2*300000);
		gpioUnset(ACT_GPIO);
		sleep_us(300000);
	}
}

void crash() {
	gpioSet(ACT_GPIO);
	gpioUnset(LED_GPIO);
	while (1) {}
}

__attribute__((naked))
__attribute__((section(".init")))
int main(void) {
	init_vector_table();
	init_stacks();
	init_process_table();
	set_irq_handler(&on_irq);
	gpioSetWay(LED_GPIO, GPIO_WAY_OUTPUT);
	gpioSetWay(ACT_GPIO, GPIO_WAY_OUTPUT);

	gpioSet(LED_GPIO);
	gpioUnset(ACT_GPIO);

	IRQ[6] = 1;
	ARM_TIMER[3] = 0;
	ARM_TIMER[7] = 0xff;
	ARM_TIMER[2] = 0x3e00a2;
	ARM_TIMER[0] = 500;
	ARM_TIMER[6] = 500;

	async_start(&led_blink, NULL);
	async_start(&act_blink, NULL);
	enable_irq();
	async_go();
}

