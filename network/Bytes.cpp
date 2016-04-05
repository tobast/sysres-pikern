#include "Bytes.h"

Bytes::Bytes() : data(nullptr), len(0), reserved(0) {}

char& Bytes::operator[](const size_t pos) {
	if(pos < len)
		return data[pos];
	throw (new OutOfRange);
}

size_t Bytes::size() const {
	return len;
}

void Bytes::append_bytes(char* bytes, size_t num) {
	if(reserved - len < num) {
		// Re-allocate 2* the space
		// TODO
	}
	for(size_t byte=0; byte < num; byte++)
		data[byte+len] = bytes[byte];
	len += num;
}

