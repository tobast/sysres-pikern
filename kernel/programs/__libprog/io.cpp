#include "io.h"
#include "syslib.h"

#include <cstdarg>

char charOfDigit(int d) {
	if(d < 10)
		return '0'+d;
	return 'A'+d-10;
}

extern "C" {
void put_int(int v, unsigned base) {
	if(v < 0) {
		putchar('-');
		v = -v;
	}
	put_uint((unsigned)v, base);
}
void put_uint(unsigned v, unsigned base) {
	if(v == 0) {
		putchar('0');
		return;
	}

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

void put_formatter(const AnsiFormat& fmt) {
	puts("\x1b[", false);
	put_uint(fmt());
	putchar('m');
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
				case 'F':
					put_formatter(va_arg(args, AnsiFormat));
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

bool str_cmp(const char* c1, const char* c2) {
	int l1 = str_len(c1), l2 = str_len(c2);
	if(l1 != l2)
		return false;
	for(int pos=0; pos < l1; pos++)
		if(c1[pos] != c2[pos])
			return false;
	return true;
}

}

