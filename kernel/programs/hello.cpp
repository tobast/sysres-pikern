#include "syslib.h"

void puts(const char *s) {
	char c;
	while (c = *s++) putchar(c);
}

int main(int argc, char** argv) {
	puts("Hello, world!\n");
}