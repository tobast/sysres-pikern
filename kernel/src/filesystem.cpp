#include "filesystem.h"
#include "fs_populator.h"
#include "process.h"

bool streq(const char* name1, const char* name2) {
	for (int i = 0; i < MAX_NAME_LENGTH + 1; i++) {
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
	char current_name[MAX_NAME_LENGTH + 1];
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
		} else if (i >= MAX_NAME_LENGTH) {
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
	uint32_t gota = (((uint32_t)d[8]) << 24) | (((uint32_t)d[9]) << 16)
	              | (((uint32_t)d[10]) << 8) | ((uint32_t)d[11]);
	uint32_t gots = (((uint32_t)d[12]) << 24) | (((uint32_t)d[13]) << 16)
	              | (((uint32_t)d[14]) << 8) | ((uint32_t)d[15]);
	const unsigned header_size = 16;
	const unsigned expected_start = 0x8000;
	char* prog = (char*)malloc(size);
	unsigned goffset = (unsigned)prog - expected_start;
	unsigned sz = file->node_file->data.size();

	for (unsigned i = header_size; i < sz; i++) {
		prog[i - header_size] = d[i];
	}
	assert(size >= sz-header_size, 0x23);
	for (unsigned i = sz - header_size; i < size; i++) {
		prog[i] = 0;
	}
	unsigned* got = (unsigned*)(prog + (gota - expected_start));
	for (unsigned i = 0; i < gots; i++) {
		got[i] += goffset;
	}
	return async_start((async_func)(prog), (void*)ec, 0x50, file->name);
}
