#pragma once

#include "common.h"

enum svcs {
	SVC_GET_PID = 1,
	SVC_READ = 2,
	SVC_WRITE = 3,
	SVC_SLEEP = 4,
	SVC_MALLOC = 5,
	SVC_FREE = 6
};

int get_pid();
int read(int fd, void* data, int num_bytes);
int write(int fd, void* data, int num_bytes);
void sleep(int num_us);
void* malloc_svc(uint32_t size);
void free_svc(void* ptr);
