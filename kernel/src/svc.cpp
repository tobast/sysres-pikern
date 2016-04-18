#include "svc.h"

__attribute__((naked))
int get_pid() {
	asm volatile ("svc #1\n\tbx lr");
}

__attribute__((naked))
int read(int, void*, int) {
	asm volatile ("svc #2\n\tbx lr");
}

__attribute__((naked))
int write(int, void*, int) {
	asm volatile ("svc #3\n\tbx lr");
}
