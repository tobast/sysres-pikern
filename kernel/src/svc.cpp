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

__attribute__((naked))
void sleep(int) {
	asm volatile ("svc #4\n\tbx lr");
}

__attribute__((naked))
void* malloc_svc(uint32_t) {
	asm volatile ("svc #5\n\tbx lr");
}

__attribute__((naked))
void free_svc(void*) {
	asm volatile ("svc #6\n\tbx lr");
}

__attribute__((naked))
int atomic_cas_svc(int*, int, int) {
	asm volatile ("svc #7\n\tbx lr");
}

__attribute__((naked))
void kill(int, int) {
	asm volatile ("svc #8\n\tbx lr");
}

__attribute__((naked))
int wait(int) {
	asm volatile ("svc #9\n\tbx lr");
}
