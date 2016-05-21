#include "format.h"
#include "malloc.h"

int readNumber(const char* str, unsigned& pos) {
	int out=0;
	while(str[pos] >= '0' && str[pos] <= '9') {
		out *= 10;
		out += str[pos]-'0';
		pos++;
	}
	return out;
}

char charOfNum(unsigned num) {
	if(num < 10)
		return '0'+num;
	return 'A'+num-10;
}

void toBase(Bytes& dest, unsigned num, int padLen, char padder, unsigned base)
{
	char buff[12];
	int pos=0;
	padLen = max(1,padLen);
	while(num != 0) {
		buff[pos] = charOfNum(num % base);
		num /= base;
		pos++;
	}
	while(pos < padLen) {
		buff[pos] = padder;
		pos++;
	}

	while(pos > 0) {
		pos--;
		dest << buff[pos];
	}
}

void toDecimal(Bytes& dest, unsigned num, int padLen, char padder) {
	toBase(dest, num, padLen, padder, 10);
}
void toHexa(Bytes& dest, unsigned num, int padLen, char padder) {
	toBase(dest, num, padLen, padder, 16);
}

void formatToBytes(Bytes& dest, const char* fmt, va_list args) {
	unsigned pos=0;
	while(fmt[pos] != '\0') {
		if(fmt[pos] == '%') {
			pos++;
			if(fmt[pos] == '%') {
				dest << '%';
			}
			else {
				char padder=' ';
				if(fmt[pos] == 0) {
					pos++;
					padder='0';
				}
				int padLen = readNumber(fmt, pos);

				switch(fmt[pos]) {
					case 'u':
						toDecimal(dest, va_arg(args,unsigned),
								padLen, padder);
						break;
					case 'X':
						toHexa(dest, va_arg(args, unsigned),
								padLen, padder);
						break;
					case 's':
						dest << va_arg(args, const char*);
						break;
					default:
						dest << "[UNKNOWN FMT: " << fmt[pos] << "]";
						break;
				}
			}
		}
		else
			dest << fmt[pos];

		pos++;
	}
}

