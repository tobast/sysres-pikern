#include "exec_context.h"

extern "C" {

void putchar(char c);
char getchar();

int get_pid();
int read(int fd, void *data, int num_bytes);
int write(int fd, void *data, int num_bytes);
void sleep(int num_us);

void* malloc(unsigned size);
void free(void *ptr);

int atomic_cas(int *p, int old_value, int new_value);
void kill(int pid, int exit_code = 0);
inline void exit(int exit_code = 0) {
	kill(get_pid(), exit_code);
}
int wait(int pid);
int is_ready_read(int fd);
int is_ready_write(int fd);
int is_process_alive(int pid);

}
