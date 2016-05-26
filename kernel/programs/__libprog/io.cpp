#include "io.h"
#include "syslib.h"

#include <cstdarg>

char charOfDigit(int d) {
	if(d < 10)
		return '0'+d;
	return 'A'+d-10;
}

void put_int(int v, unsigned base) {
	if(v < 0) {
		putchar('-');
		v = -v;
	}
	put_uint((unsigned)v, base);
}
void put_uint(unsigned v, unsigned base) {
	char buff[11];
	unsigned pos=0;
	while(v > 0) {
		buff[pos] = charOfDigit(v % base);
		v /= base;
		pos++;
	}
	while(pos > 0) {
		pos--;
		putchar(buff[pos]);
	}
}

void puts(const char* v, bool endl) {
	for(unsigned pos=0; v[pos] != '\0'; pos++)
		putchar(v[pos]);
	if(endl)
		putchar('\n');
}

void printf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	for(unsigned pos=0; fmt[pos] != '\0'; pos++) {
		if(fmt[pos] == '%') {
			pos++;
			switch(fmt[pos]) {
				case 's':
					puts(va_arg(args, const char*), false);
					break;
				case 'd':
					put_int(va_arg(args, int));
					break;
				case 'u':
					put_uint(va_arg(args, unsigned));
					break;
				case 'X':
					put_uint(va_arg(args, unsigned), 16);
					break;
				case 'c':
					putchar(va_arg(args, int));
					break;
				default:
					puts("[UNKNOWN FORMATTER]", false);
					break;
			}
		}
		else
			putchar(fmt[pos]);
	}

	va_end(args);
}

