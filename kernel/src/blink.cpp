#include <cstdint>
typedef int32_t Int;
//typedef int Int;
#define GPIO	((Int volatile *)0x20200000)
#define TIMER ((Int volatile *)0x20003000)
const char GPIO_WAY_INPUT =		0b000;
const char GPIO_WAY_OUTPUT =	0b001;

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

__attribute__((naked))
__attribute__((section(".init")))
int main(void) {
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

