#pragma once

#include "common.h"

void init_process_table();

// Add a process before active_process
void add_process(int i);

void delete_process(int i);

int create_process();

void next_process();

typedef void (*async_func)(void*);

void async_start(async_func f, void* arg);

void async_go();
