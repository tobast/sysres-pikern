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
	if(num == 0) {
		buff[pos] = '0';
		pos++;
	}
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
void toSignedDecimal(Bytes& dest, int num, int padLen, char padder) {
	if(num < 0) {
		dest << '-';
		toBase(dest, -num, padLen, padder, 10);
	}
	else
		toBase(dest, num, padLen, padder, 10);
}
void toHexa(Bytes& dest, unsigned num, int padLen, char padder) {
	toBase(dest, num, padLen, padder, 16);
}

void toMacAddr(Bytes& dest, HwAddr mac) {
	for(int byte=0; byte < 6; byte++) {
		toHexa(dest, (mac >> 8*byte) & 0xff, 2, '0');
		if(byte < 5)
			dest << ':';
	}
}

void toIpv4Addr(Bytes& dest, Ipv4Addr addr) {
	for(int byte=3; byte >= 0; byte--) {
		toDecimal(dest, (addr >> 8*byte) & 0xff, 0, ' ');
		if(byte > 0)
			dest << '.';
	}
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
				if(fmt[pos] == '0') {
					pos++;
					padder='0';
				}
				int padLen = readNumber(fmt, pos);

				switch(fmt[pos]) {
					case 'u':
						toDecimal(dest, va_arg(args,unsigned),
								padLen, padder);
						break;
					case 'd':
						toSignedDecimal(dest, va_arg(args,int),
								padLen, padder);
						break;
					case 'X':
						toHexa(dest, va_arg(args, unsigned),
								padLen, padder);
						break;
					case 's':
						dest << va_arg(args, const char*);
						break;
					case 'M': // MAC address
						toMacAddr(dest, va_arg(args, HwAddr));
						break;
					case 'I': // IPv4 address
						toIpv4Addr(dest, va_arg(args, Ipv4Addr));
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

