#include "exec_context.h"

extern "C" {

void putchar(char c);
char getchar();

int get_pid();

// Reads up to num_bytes of data from socket fd, to data.
// Returns the number of bytes actually read.
int read(int fd, void *data, int num_bytes);

// Writes up to num_bytes of data to socket fd.
// Returns the number of bytes actually written.
int write(int fd, void *data, int num_bytes);

// Sleeps at least num_us microseconds.
void sleep(int num_us);

void* malloc(unsigned size);
void free(void *ptr);

// Perform an atomic compare-and-swap operation; returns the old value.
int atomic_cas(int *p, int old_value, int new_value);

// Kill process with PID pid, with exit code exit_code.
void kill(int pid, int exit_code = 0);

// Exit the process with exit code exit_code.
inline void exit(int exit_code = 0) {
	kill(get_pid(), exit_code);
	while (true);
}

// Wait for process pid to terminate; return its exit code.
int wait(int pid);

// Checks if socket fd is ready for reading.
int is_ready_read(int fd);

// Checks if socket fd is ready for writing.
int is_ready_write(int fd);

// Returns true if a process with PID pid is alive.
int is_process_alive(int pid);

// Returns a file handle to the file called name.
int find_file(const char* name);

// Returns the size of the file file.
int file_size(int file);

// Reads up to length bytes of data in file file, at offset offset.
int file_read(int file, int offset, void* addr, int length);

}
