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

int main(int argc, char** argv) {
	kill(atoi(argv[1]), 1);
}

