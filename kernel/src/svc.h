#pragma once

#include "common.h"

enum svcs {
	SVC_GET_PID = 1,
	SVC_READ = 2,
	SVC_WRITE = 3,
	SVC_SLEEP = 4,
	SVC_MALLOC = 5,
	SVC_FREE = 6,
	SVC_ATOMIC_CAS = 7,
	SVC_KILL = 8,
	SVC_WAIT = 9,
	SVC_READY_READ = 10,
	SVC_READY_WRITE = 11,
	SVC_PROCESS_ALIVE = 12,
	SVC_FIND_FILE = 13,
	SVC_FILE_SIZE = 14,
	SVC_FILE_READ = 15
};

extern "C" {

int get_pid();
int read(int fd, void* data, int num_bytes);
int write(int fd, void* data, int num_bytes);
void sleep(int num_us);
void* malloc_svc(uint32_t size);
void free_svc(void* ptr);
int atomic_cas_svc(int* p, int old_value, int new_value);
void kill(int pid, int exit_code = 0);
inline void exit(int exit_code = 0) {
	kill(get_pid(), exit_code);
	while(true);
}
int wait(int pid);
int is_ready_read(int fd);
int is_ready_write(int fd);
int is_process_alive(int pid);
int find_file(const char* name);
int file_size(int file);
int file_read(int file, int offset, void* addr, int length);

}
