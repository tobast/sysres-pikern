#include "malloc.h"

#include "gpio.h"

const uint32_t MEMORY_START(0x2000000), MEMORY_END(0x10000000);
uint32_t brk(MEMORY_START);

void mallocInit() {
	brk = MEMORY_START;
}

void* malloc(unsigned size) {
	// For now, this is just a SBRK function.
	assert(brk + size <= MEMORY_END);
	
	void* out = (void*)brk;
	brk += size;
	brk += (-brk) & 3; // Aligns to 4-bytes.
	return out;
}

void free(void*) {
}

