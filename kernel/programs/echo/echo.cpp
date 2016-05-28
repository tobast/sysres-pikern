#include "syslib.h"
#include "proglib.h"

int main(int argc, char** argv) {
	for(int arg=1; arg < argc; arg++) {
		puts(argv[arg], false);
		if(arg != argc-1)
			putchar(' ');
	}
	int lastLen = str_len(argv[argc-1]);
	if(lastLen == 0 || argv[argc-1][lastLen-1] != '\n')
		putchar('\n');
	return 0;
}

