#pragma once

#include "exec_context.h"
#include "udpSysWrite.h"

extern "C" {

void putchar(char c);
char getchar();

int get_stdin();
int get_stdout();

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

// Returns a file handle to the file called name, or 0 if inexistant.
int find_file(const char* name);

// Returns the size of the file file.
int file_size(int file);

// Reads up to length bytes of data in file file, at offset offset.
int file_read(int file, int offset, void* addr, int length);

// Gets the number of active processes.
int nb_processes();

// Get the PIDs of all processes in data, with a maximum number
// of max_processes processes.
// Returns the number of processes actually written.
int get_processes(int* data, int max_processes);

// Writes the name of process with PID pid in name.
// name should be at least 32 bytes long.
void get_process_name(int pid, char* name);

// Returns the state of process pid.
char get_process_state(int pid);

// Writes the given UdpSysData to the network. Returns the number of bytes
// effectively written.
int udp_write(UdpSysData* data);

// Binds the given UDP port. Returns a socket handle, or -1 if an error occured.
int udp_bind(uint16_t port);

// Reads up to [max_len] bytes from [udpHandle] to [buff], returning
// the address and port the packet came from, and its size.
int udp_read(int udpHandle, void* buff, unsigned maxLen, UdpSysRead* out);

// Execute a file, in the following execution context.
// Returns the pid of the process, or -1 if the file could not be executed.
int execute_file(int file, execution_context *ec);

// Returns 1 if node is a folder
int is_folder(int node);

// Returns the number of children of folder
int num_children(int folder);

// Returns the child number child_num of folder
// Returns 0 on failure
int get_child(int folder, int child_num);

// Copies the name of node to buffer, with a maximum of max_chars
// characters.
void get_node_name(int node, char* buffer, int max_chars);

// Creates a socket, and returns its identifier
int new_socket();

// Returns the handle to the current working directory.
int get_cwd();

// Sets the current working directory to node with the provided handle.
void set_cwd(int node);

// Returns the handle to the parent of node. This can be 0 if
// node is the root of the filesystem.
int get_parent(int node);

}

