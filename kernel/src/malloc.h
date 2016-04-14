#ifndef DEF_KER_MALLOC
#define DEF_KER_MALLOC

#include <cstdlib>

#include "assert.h"

void* malloc(unsigned size);
/// Allocates a block of [size] bytes in memory, returns a pointer to the first
/// byte of the allocated block.
/// Crashes the system whenever the memory limit is reached.

void free(void* ptr);
/// Frees previously allocated memory.

#endif//DEF_KER_MALLOC
