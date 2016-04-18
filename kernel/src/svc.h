#pragma once

#include "common.h"

enum svcs {
	SVC_GET_PID = 1,
	SVC_READ = 2,
	SVC_WRITE = 3
};

inline int get_pid() {
	asm volatile ("svc #1");
}

inline int read(int fd, void* data, int num_bytes) {
	asm volatile ("svc #2");
}

inline int write(int fd, void* data, int num_bytes) {
	asm volatile ("svc #3");
}
