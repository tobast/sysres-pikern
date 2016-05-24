#!/usr/bin/python3

input_folder = "./initial_fs"
output_file = "./_build/fs_populator.cpp"

import os

prelude = """
#include "fs_populator.h"
#include "malloc.h"

file* create_file(const char* raw_data, unsigned length) {
	file* f = (file*)(malloc(sizeof(file)));
	f->data.init();
	f->data.reserve(length);
	for (unsigned i = 0; i < length; i++) {
		f->data.push_back(raw_data[i]);
	}
	return f;
}

void set_name(node *n, const char* name, unsigned length) {
	for (unsigned i = 0; i < length; i++) {
		n->name[i] = name[i];
	}
}

void populate_fs(folder *fsroot) {

"""

def file_to_string(name):
	f = open(name, "rb")
	d = f.read()
	f.close()
	return len(d), dump_bytes(d)

def dump_string(d):
	return '"' + ''.join(c if 32 <= ord(c) <= 126 and c != '"' and c != '\\' else "\\x%02x"%ord(c) for c in d) + '"'

def dump_bytes(d):
	return '"' + ''.join(chr(c) if 32 <= c <= 126 and c != ord('"') and c != ord('\\') else "\\x%02x"%c for c in d) + '"'


out = prelude
for dirpath, dirnames, filenames in os.walk(input_folder):
	assert(dirpath.startswith(input_folder))
	ad = dirpath[len(input_folder):]
	varname = "fsroot" + ad.replace("/", "__")
	for fname in filenames:
		vvname = varname + "___" + fname
		rname = os.path.join(dirpath, fname)
		k, s = file_to_string(rname)
		assert (len(fname) <= 31)
		out += "node* %s = (node*)(malloc(sizeof(node)));\n" % vvname
		out += "%s->type = NODE_FILE;\n" % vvname
		out += "set_name(%s, %s, %d);\n" % (vvname, dump_string(fname), len(fname) + 1)
		out += "%s->node_file = create_file(%s, %d);\n" % (vvname, s, k)
		out += "%s->contents.push_back(%s);\n" % (varname, vvname)
		out += "\n"

	for dname in dirnames:
		vvname = varname + "___" + dname
		vfname = varname + "__" + dname
		assert (len(dname) <= 31)
		out += "node* %s = (node*)(malloc(sizeof(node)));\n" % vvname
		out += "%s->type = NODE_FOLDER;\n" % vvname
		out += "set_name(%s, %s, %d);\n" % (vvname, dump_string(dname), len(dname) + 1)
		out += "%s->node_folder = (folder*)(malloc(sizeof(folder)));\n" % vvname
		out += "folder* %s = %s->node_folder;\n" % (vfname, vvname)
		out += "%s->contents.init();\n" % vfname
		out += "%s->contents.push_back(%s);\n" % (varname, vvname)
		out += "\n"

out += "}\n"

f = open(output_file, "w")
f.write(out)
f.close()
	
	