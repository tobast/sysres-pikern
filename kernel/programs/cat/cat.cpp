#include "syslib.h"
#include "proglib.h"

void catFile(const char* path) {
	int handle = find_file(path);
	if(handle == 0) {
		printf("%s: no such file or directory.\n", path);
		return;
	}

	const int BUFF_SIZE=1024;
	char buffer[BUFF_SIZE];
	int nbChunks = (file_size(handle)+BUFF_SIZE-1)/BUFF_SIZE;
	int curSize;
	for(int chunk=0; chunk < nbChunks; chunk++) {
		curSize = file_read(handle, chunk*BUFF_SIZE, buffer, BUFF_SIZE);
		for(int pos=0; pos < curSize; pos++)
			putchar(buffer[pos]);
	}
	if(buffer[curSize-1] != '\n')
		putchar('\n');
}

int main(int argc, char** argv) {
	if(argc < 2) { // echo mode
		while (true) {
			char c = getchar();
			putchar(c);
		}
	}

	else {
		for(int arg=1; arg < argc; arg++)
			catFile(argv[arg]);
	}

	return 0;
}

