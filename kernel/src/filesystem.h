#pragma once

#include "malloc.h"
#include "expArray.h"
#include "common.h"
#include "exec_context.h"

struct file {
	ExpArray<char> data;
};

struct node;
struct folder {
	ExpArray<node*> contents;
};

enum node_type {
	NODE_FILE,
	NODE_FOLDER
};

struct node {
	char name[32];
	node_type type;
	union {
		file *node_file;
		folder *node_folder;
	};
};

void init_filesystem();
node* find_child(const folder *f, const char* name);
node* follow_path(const char* path);
int run_process(node *file, execution_context *ec);
