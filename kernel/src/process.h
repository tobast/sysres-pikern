#pragma once

#include "common.h"
#include "expArray.h"
#include "genericSocket.h"
#include "udpSocket.h"

void init_process_table();

// Create a new socket
int create_socket(GenericSocket *s = NULL);
int create_udp_socket(UdpSocket *s = NULL);

// Close the socket number i
void close_socket(int i);
void close_udp_socket(int i);

// Add a process before active_process
void add_process(int i);

void delete_process(int i);

int create_process();

void next_process();

typedef void (*async_func)(void*);

// Default mode : user mode, enable IRQ, disable FIQ)
int async_start(async_func f, void* arg, s32 mode = 0x50,
		const char* name = NULL, int cwd = 0);

ExpArray<int> alive_processes();

const char* process_name(int pid);
char process_state(int pid);

void async_go();
