#pragma once

#include "io.h"
#include "fs.h"
#include "hashTable.h"
#include "syslib.h"
#include "expArray.h"
inline void _assert_fail(char const * file, char const * func,
		int line, char const * expr, unsigned char)
{
	printf("In file \"%s\", line %d, in function %s:\n"
		"Assertion '%s' failed.\n", file, line, func, expr);
	exit(1);
}

