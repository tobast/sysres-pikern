#include "genericSocket.h"
#include "assert.h"

GenericSocket::GenericSocket(bool blockingState, unsigned maxSize) :
	blocking(blockingState), maxSize(maxSize), sinceLastDelim(0)
{}

unsigned GenericSocket::write(const void* inData, unsigned len, bool addDelim,
		bool writeWhole)
{
	if(len == 0)
		return 0;
	unsigned actualLen = min(len, maxSize - data.size());
	if(writeWhole && actualLen != len)
		return 0;

	// Force at least one loop turn, to run writeByte's checks
	for(unsigned pos=0; pos < max(1u, actualLen); pos++)
		writeByte(((uint8_t*)inData)[pos]);
	if(addDelim && actualLen == len)
		writeDelimiter();
	return actualLen;
}
void GenericSocket::writeByte(uint8_t v) {
	if(isFull()) {
		assert(!blocking, 0);
		// Else, we're a non-blocking sock
		return;
	}
	sinceLastDelim++;
	data.push(v);
}
void GenericSocket::writeDelimiter() {
	delims.push(sinceLastDelim);
	sinceLastDelim = 0;
}

unsigned GenericSocket::read(void* buff, unsigned maxSize, bool* atDelim) {
	unsigned len = min(maxSize, data.size());
	if(!delims.empty() && delims.front() == 0)
		delims.pop();

	for(unsigned pos=0; pos < len; pos++) {
		if(!delims.empty() && delims.front() == 0) {
			delims.pop();
			if(atDelim != NULL)
				*atDelim = true;
			return pos;
		}
		((uint8_t*)buff)[pos] = readByte();
	}

	if(!delims.empty() && delims.front() == 0) {
		delims.pop();
		if(atDelim != NULL)
			*atDelim = true;
	}
	else if(atDelim != NULL)
		*atDelim = false;
	return len;
}
uint8_t GenericSocket::readByte() {
	if(isEmpty())
		throw Empty();

	if(delims.empty())
		sinceLastDelim--;
	else {
		assert(delims.front() != 0, 0);
		delims.front()--;
	}

	return data.pop();
}

bool GenericSocket::isFull() const {
	return data.size() == maxSize;
}
bool GenericSocket::isEmpty() const {
	return data.size() == 0;
}
bool GenericSocket::isBlocking() const {
	return blocking;
}

