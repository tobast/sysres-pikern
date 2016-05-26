#include "syslib.h"
#include "proglib.h"
#include <cstdint>

const unsigned PROCESS_MAX_NAME = 64;

int main(int, char**) {
	unsigned nbProc = nb_processes();
	int* pids = (int*)malloc(nbProc * sizeof(int));
	get_processes(pids, nbProc);

	for(unsigned proc=0; proc < nbProc; proc++) {
		char pName[PROCESS_MAX_NAME];
		get_process_name(pids[proc], pName);
		printf("%d\t| %c | %s\n", pids[proc],
				get_process_state(pids[proc]), pName);
	}

	free(pids);
	return 0;
}

