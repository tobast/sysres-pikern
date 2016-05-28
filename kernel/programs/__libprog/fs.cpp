#include "fs.h"

void get_node_path(int node, char* buffer, int max_chars) {
	if(node == 0) {
		buffer[0] = '\0';
		return;
	}

	get_node_path(get_parent(node), buffer, max_chars);
	int len = str_len(buffer);
	get_node_name(node, buffer+len, max_chars-1-len);
	len = str_len(buffer+len)+len;
	buffer[len] = '/';
	buffer[len+1] = '\0';
}

