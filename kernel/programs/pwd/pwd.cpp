#include "syslib.h"
#include "proglib.h"

int main(int, char**) {
	int cwdHandle = get_cwd();
	char buffer[256];
	get_node_path(cwdHandle, buffer, 256);

	puts(buffer);
	return 0;
}

