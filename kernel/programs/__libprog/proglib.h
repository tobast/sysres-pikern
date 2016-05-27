#include "io.h"
#include "hashTable.h"
#include "expArray.h"
inline void _assert(bool b, char const * file, char const * func,
	int line, char const * expr, unsigned char) {
	if (!b) {
	    printf("In file \"%s\", line %d, in function %s:\n"
			"Assertion '%s' failed.\n", file, line, func, expr);
		exit(1);
	}
}

