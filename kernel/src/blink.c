#define GPIO	((void*)0x20200000)
const char GPIO_WAY_INPUT =		0b000;
const char GPIO_WAY_OUTPUT =	0b001;

inline void gpioSet(int i) {
	// Sets the bit i of GPIO + 0x1C
	int orMask = (1<<(i%32));
	*((int*)(GPIO+0x1C+(i/32))) = orMask;
}
inline void gpioUnset(int i) {
	// Sets the bit i of GPIO + 0x28
	int orMask = (1<<(i%32));
	*((int*)(GPIO+0x28+(i/32))) = orMask;
}
inline void gpioSetWay(int i, int way) {
	// Sets the 3 bits at position 3*i%10 of (GPIO+4*i//10) to way
	int orMask = (way<<(3*(i%10)));
	*((int*)(GPIO+4*(i/10))) = orMask;
}

void sleep(unsigned int cyc) {
	for(; cyc > 0; --cyc);
}

int main(void) __attribute__((naked)); // Never return.
int main(void) {
	const int LED_GPIO = 25;
	gpioSetWay(LED_GPIO, GPIO_WAY_OUTPUT);

	while(1) {
		gpioSet(LED_GPIO);
		sleep(0x3F0000);
		gpioUnset(LED_GPIO);
		sleep(0x3F0000);
	}
}

