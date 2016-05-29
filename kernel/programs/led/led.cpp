#include "syslib.h"
#include "proglib.h"

int atoi(const char* v) {
	int out=0;
	for(int pos=0; v[pos] >= '0' && v[pos] <= '9' && pos < 18; pos++) {
		out *= 10;
		out += v[pos] - '0';
	}
	return out;
}

void setBit(int bit, bool val) {
	if(bit < 0 || bit >= 8) {
		printf("Invalid LED id: %d\n", bit);
		exit(1);
	}
	gpio_set(bit, val);
}

void dispByte(int val) {
	if(val > 255) {
		printf("Too large: %d\n", val);
		exit(1);
	}

	for(int pin=0; pin < 8; pin++) {
		setBit(pin, val % 2 == 1);
		val /= 2;
	}
}

void dispXMas(int turns) {
	if(turns > 42) {
		puts("Sorry, but it won't be Christmas forever.");
		exit(1);
	}

	int num = 0x7;
	for(int i=0; i < 8*turns; i++) {
		dispByte(num);
		num <<= 1;
		if(num > 0xff)
			num = (num & 0xff) + 1;
		sleep(50*1000);
	}
	dispByte(0);
}

int main(int argc, char** argv) {
	if(argc < 3) {
		if(argc == 0)
			puts("Missing argument.");
		else
			printf("Missing argument. Usage:\n%s [on|off] [0..7]\n"
				"%s int [0..255]\n", argv[0], argv[0]);
		exit(1);
	}

	if(str_cmp(argv[1], "int"))
		dispByte(atoi(argv[2]));
	else if(str_cmp(argv[1],"on"))
		setBit(atoi(argv[2]), true);
	else if(str_cmp(argv[1], "off"))
		setBit(atoi(argv[2]), false);
	else if(str_cmp(argv[1], "xmas"))
		dispXMas(atoi(argv[2]));
	else {
		printf("Invalid action: %s\n", argv[1]);
		exit(1);
	}
	return 0;
}

