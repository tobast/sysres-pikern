#pragma once

#include "common.h"

void init_process_table();

// Create a new socket
int create_socket();

// Close the socket number i
void close_socket(int i);

// Add a process before active_process
void add_process(int i);

void delete_process(int i);

int create_process();

void next_process();

typedef void (*async_func)(void*);

// Default mode : user mode, enable IRQ, disable FIQ)
void async_start(async_func f, void* arg, s32 mode = 0x50);

void async_go();
