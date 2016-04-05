#include <cstdint>
typedef int32_t Int;
//typedef int Int;
#define GPIO	((Int*)0x20200000)
const char GPIO_WAY_INPUT =		0b000;
const char GPIO_WAY_OUTPUT =	0b001;

inline void gpioSet(Int i) {
	// Sets the bit i of GPIO + 0x1C
	Int orMask = (1<<(i%32));
	*(GPIO+0x1C+(i/32)) = orMask;
}
inline void gpioUnset(Int i) {
	// Sets the bit i of GPIO + 0x28
	Int orMask = (1<<(i%32));
	*(GPIO+0x28+(i/32)) = orMask;
}
inline void gpioSetWay(Int i, Int way) {
	// Sets the 3 bits at position 3*i%10 of (GPIO+4*i//10) to way
	Int orMask = (way<<(3*(i%10)));
	*(GPIO+(i/10)) = orMask;
}

void sleep(Int cyc) {
	for(; cyc > 0; --cyc);
}

int main(void) __attribute__((naked)); // Never return.
int main(void) {
	const Int LED_GPIO = 25, ACT_GPIO = 16;
	gpioSetWay(LED_GPIO, GPIO_WAY_OUTPUT);
	gpioSetWay(ACT_GPIO, GPIO_WAY_OUTPUT);

	while(1) {
		gpioSet(LED_GPIO);
		gpioUnset(ACT_GPIO);
		sleep(0x3F0000);
		gpioUnset(LED_GPIO);
		gpioSet(ACT_GPIO);
		sleep(0x3F0000);
	}
}

