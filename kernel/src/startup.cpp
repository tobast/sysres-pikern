#include "exec_context.h"
#include "svc.h"

int main(int, char**);

extern "C" {

int _stdin_socket;
int _stdout_socket;

__attribute__((section(".init")))
void _start(void* args) {
	execution_context *ec = (execution_context*)args;
	_stdin_socket = ec->stdin;
	_stdout_socket = ec->stdout;
	exit(main(ec->argc, ec->argv));
}

void putchar(char c) {
	write(_stdout_socket, (void*)(&c), 1);
}
char getchar() {
	char c;
	read(_stdin_socket, (void*)(&c), 1);
	return c;
}

void* malloc(unsigned size) {
	return malloc_svc(size);
}

void free(void* ptr) {
	free_svc(ptr);
}

int atomic_cas(int *p, int old_value, int new_value) {
	return atomic_cas_svc(p, old_value, new_value);
}

}
