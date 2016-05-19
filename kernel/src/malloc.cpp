#include "malloc.h"

#include "gpio.h"
#include "svc.h"
#include "interrupts.h"

const uint32_t MEMORY_START(0x2000000), MEMORY_END(0x10000000);
uint32_t brk(MEMORY_START);

void mallocInit() {
	brk = MEMORY_START;
}

void* malloc_nocheck(uint32_t size) {
	// For now, this is just a SBRK function.
	assert(brk + size <= MEMORY_END);
	
	void* out = (void*)brk;
	brk += size;
	brk += (-brk) & 3; // Aligns to 4-bytes.
	return out;
}

void free_nocheck(void*) {
}

void* malloc(unsigned size) {
	u32 cpsr = get_cpsr();
	if ((cpsr & 0x1f) == 0x12) {
		return malloc_nocheck(size);
	}
	return malloc_svc(size);
}

void free(void* ptr) {
	u32 cpsr = get_cpsr();
	if ((cpsr & 0x1f) == 0x12) {
		return free_nocheck(ptr);
	}
	return free_svc(ptr);
}

