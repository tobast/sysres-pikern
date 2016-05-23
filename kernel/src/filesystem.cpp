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
