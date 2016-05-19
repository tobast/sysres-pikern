#ifndef DEF_KER_MALLOC
#define DEF_KER_MALLOC

#include <cstdint>

#include "assert.h"

void mallocInit();

void* malloc_nocheck(uint32_t size);
void free_nocheck(void* ptr);

void* malloc(uint32_t size);
/// Allocates a block of [size] bytes in memory, returns a pointer to the first
/// byte of the allocated block.
/// Crashes the system whenever the memory limit is reached.

void free(void* ptr);
/// Frees previously allocated memory.

#endif//DEF_KER_MALLOC
