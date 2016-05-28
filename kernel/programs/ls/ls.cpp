#include "syslib.h"
#include "proglib.h"

void dispLsEntry(int handle) {
	char name[256];
	get_node_name(handle, name, 256);

	if(is_folder(handle))
		printf("%F%F%s%F", AnsiFormat(ANSI_BOLD),
				AnsiFormat(ANSI_FG, ANSI_BLUE),
				name, AnsiFormat());
	else
		printf("%s", name);
}

int main(int argc, char** argv) {
	if(argc < 2) {
		puts("You must specify a directory, or ask Nathanaël "
				"for a 'working directory' concept.");
		exit(1);
	}

	int dirHandle = find_file(argv[1]);
	if(dirHandle == 0) {
		printf("No such file or directory: %s.\n", argv[1]);
		exit(1);
	}

	if(!is_folder(dirHandle)) {
		printf("Not a directory: %s.\n", argv[1]);
		exit(1);
	}

	int nbChildren = num_children(dirHandle);
	for(int chId=0; chId < nbChildren; chId++) {
		dispLsEntry(get_child(dirHandle, chId));
		puts("");
	}

	return 0;
}

