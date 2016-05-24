#include "filesystem.h"
#include "fs_populator.h"
#include "process.h"

bool streq(const char* name1, const char* name2) {
	for (int i = 0; i < 32; i++) {
		if (name1[i] != name2[i])
			return false;
		else if (name1[i] == '\0')
			return true;
	}
	return true;
}

folder fsroot;
void init_filesystem() {
	fsroot.contents.init();
	populate_fs(&fsroot);
}

node* find_child(const folder *f, const char* name) {
	u32 n = f->contents.size();
	for (u32 i = 0; i < n; i++) {
		if (streq(f->contents[i]->name, name))
			return f->contents[i];
	}
	return NULL;
}

node* follow_path(const char* path) {
	char current_name[32];
	u32 index = 0;
	u32 i = 0;
	folder *current_folder = &fsroot;
	while (true) {
		char c = path[index++];
		if (c == '\0') {
			break;
		} else if (c == '/') {
			current_name[i++] = '\0';
			node *n = find_child(current_folder, current_name);
			if (n == NULL) return NULL;
			if (n->type != NODE_FOLDER) {
				return NULL;
			}
			if (path[index] == '\0') {
				return n;
			}
			current_folder = n->node_folder;
			i = 0;
		} else if (i >= 31) {
			return NULL;
		} else {
			current_name[i++] = c;
		}
	}
	current_name[i++] = '\0';
	return find_child(current_folder, current_name);
}

int run_process(node *file, execution_context *ec) {
	if (file->type != NODE_FILE) return -1;
	char* d = &file->node_file->data[0];
	if (d[0] != '\x7f' || d[1] != 'E' || d[2] != 'L' || d[3] != 'F') {
		return -1;
	}
	uint32_t size = (((uint32_t)d[4]) << 24) | (((uint32_t)d[5]) << 16)
	              | (((uint32_t)d[6]) << 8) | ((uint32_t)d[7]);
	char* prog = (char*)malloc(size);
	unsigned sz = file->node_file->data.size();
	for (unsigned i = 8; i < sz; i++) {
		prog[i - 8] = d[i];
	}
	for (unsigned i = sz - 8; i < size; i++) {
		prog[i] = 0;
	}
	return async_start((async_func)(prog), (void*)ec, 0x50, file->name);
}
