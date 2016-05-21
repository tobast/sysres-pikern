#include "malloc.h"

#include "gpio.h"
#include "svc.h"
#include "interrupts.h"

const uint32_t MEMORY_START(0x2000000), MEMORY_END(0x10000000);
const uint32_t MEMORY_ALIGN_MASK = 0xf;
uint32_t brk(MEMORY_START);

void mallocInit() {
	brk = MEMORY_START;
}

void* malloc_nocheck(uint32_t size) {
	// For now, this is just a SBRK function.
	assert(brk + size <= MEMORY_END, 0xaa);
	
	brk += (-brk) & MEMORY_ALIGN_MASK; // Aligns to 16-bytes.
	assert((brk & MEMORY_ALIGN_MASK) == 0, 0xab);
	void* out = (void*)brk;
	brk += size;

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

