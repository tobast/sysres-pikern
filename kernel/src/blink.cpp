#include <cstdint>
typedef int32_t Int;
//typedef int Int;
#define GPIO	((Int volatile *)0x20200000)
#define TIMER ((Int volatile *)0x20003000)
const char GPIO_WAY_INPUT =		0b000;
const char GPIO_WAY_OUTPUT =	0b001;

typedef void (*interrupt)(Int*);
extern interrupt _interrupt_handler_addr;
interrupt volatile * const interrupt_handler_addr = &_interrupt_handler_addr; 
extern interrupt _irq_handler_addr;
interrupt volatile * const irq_handler_addr = &_irq_handler_addr;
extern Int _vector_table;
Int volatile * const vector_table = &_vector_table;
Int volatile * const vector_table_dst = 0;
const Int vector_table_length = 15;

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
	"b _asm_irq_handler\n\t" // irq
	"ldr pc, _interrupt_handler_addr\n\t" // fiq
"_interrupt_handler_addr:\n\t"
	".space 4\n\t"
"_irq_handler_addr:\n\t"
	".space 4\n\t"
"_asm_irq_handler:\n\t"
	"push {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}\n\t" // Save registers
	"ldr r0, _irq_handler_addr\n\t"
    "blx r0\n\t"
    "pop  {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}\n\t" // Restore them
    "subs pc,lr,#4\n\t"
);

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
}

void set_irq_handler(interrupt handler) {
	*((interrupt*)((Int)vector_table_dst + (Int)irq_handler_addr - (Int)vector_table)) = handler;
}

void on_interrupt(Int* args) {
	
}

__attribute__((naked))
__attribute__((section(".init")))
int main(void) {
	init_vector_table();
	set_irq_handler(&on_interrupt);
	const Int LED_GPIO = 25, ACT_GPIO = 16;
	gpioSetWay(LED_GPIO, GPIO_WAY_OUTPUT);
	gpioSetWay(ACT_GPIO, GPIO_WAY_OUTPUT);

	while(1) {
		gpioSet(LED_GPIO);
		gpioSet(ACT_GPIO);
		sleep_us(2*500000);
		gpioUnset(LED_GPIO);
		gpioUnset(ACT_GPIO);
		sleep_us(500000);
	}
}

