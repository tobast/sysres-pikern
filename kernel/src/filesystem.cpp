#include "filesystem.h"
#include "fs_populator.h"

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
