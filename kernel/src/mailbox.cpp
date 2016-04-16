#include "mailbox.h"
#include "malloc.h"
#include "sleep.h"
#include "hardware_constants.h"

namespace mailbox {

// ========= MAILBOX CONSTANTS ===========
const uint32_t RESP_SUCCESS = 0x80000000;
const uint32_t RESP_ERR_PARSE = 0x80000001;

const uint32_t STATUS_FULL = 0x80000000;
const uint32_t STATUS_EMPTY = 0x40000000;

// Tag values
const uint32_t VAL_MODEL = 0x00010001;
const uint32_t VAL_REV = 0x00010002;
const uint32_t VAL_MACADDR = 0x00010003;
const uint32_t VAL_RAMSIZE = 0x00010005;
// ===== END MAILBOX CONSTANTS ===========

typedef uint32_t Ptr;

void checkAlignment(uint32_t* buffer) {
	if(((Ptr)buffer & 0xFFFFFFF0) != (Ptr)buffer)
		throw new WrongAlignment;
}

uint32_t* initBuffer(uint32_t* buffer) {
	/// Returns the next position to write, as an uint32_t*
	checkAlignment(buffer);
	buffer[0] = 0x00000000;
	buffer[1] = 0x00000000;
	return (uint32_t*)(buffer+2);
}

/// Closes the tag [buffer], which next free-to-write position is [pos].
void closeBuffer(uint32_t* buffer, uint8_t* pos) {
	// End tag (0x00000000)
	pos[0] = 0x00;
	pos[1] = 0x00;
	pos[2] = 0x00;
	pos[3] = 0x00;

	// Pad to 16-bytes aligned.
	pos += 4;
	while((Ptr)pos % 16 != 0) {
		*pos = 0x00;
		pos++;
	}

	// Tag size
	buffer[0] = (uint32_t)pos - (uint32_t)buffer;
}

uint32_t* writeTag(uint32_t* buffer, uint32_t tagId,
		size_t bufLen, size_t valLen, uint8_t* data)
	/// Returns the next writeable position.
{
	buffer[0] = tagId;
	buffer[1] = bufLen;
	buffer[2] = 0x7fffffff & valLen;

	uint8_t* valBuffer = (uint8_t*)(buffer+3);
	for(size_t pos=0; pos < valLen; pos++)
		valBuffer[pos] = data[pos];
	for(size_t pos=valLen; pos < bufLen; pos++)
		valBuffer[pos] = 0x00;
	
	return (uint32_t*) ((uint8_t*)(buffer + 3) + bufLen);
}

void buildMacRequest(uint32_t* buffer) {
	uint32_t* writeAddr = initBuffer(buffer);
	writeAddr = writeTag(writeAddr, VAL_MACADDR, 6, 0, NULL);
	closeBuffer(buffer, (uint8_t*)writeAddr);
}
void buildModelRequest(uint32_t* buffer) {
	uint32_t* writeAddr = initBuffer(buffer);
	writeAddr = writeTag(writeAddr, VAL_MODEL, 4, 0, NULL);
	closeBuffer(buffer, (uint8_t*)writeAddr);
}
void buildRevisionRequest(uint32_t* buffer) {
	uint32_t* writeAddr = initBuffer(buffer);
	writeAddr = writeTag(writeAddr, VAL_REV, 4, 0, NULL);
	closeBuffer(buffer, (uint8_t*)writeAddr);
}
void buildTotalRamRequest(uint32_t* buffer) {
	uint32_t* writeAddr = initBuffer(buffer);
	writeAddr = writeTag(writeAddr, VAL_RAMSIZE, 8, 0, NULL);
	closeBuffer(buffer, (uint8_t*)writeAddr);
}

void sleepWithCrash(int time, uint32_t timeout, uint32_t& slept) {
	assert(timeout == 0 || slept < timeout);
	sleep_us(time);
	slept += time;
}

void readTag(uint32_t* buffer, uint32_t timeout) {
	static const int SLEEP_DELAY = 5;

	checkAlignment(buffer); // If the alignment is wrong, we could hang forever

	uint32_t slept=0;
	// Wait for the buffer to be writeable
	while((hardware::mailbox::STATUS[0] & STATUS_FULL) != 0) {
		sleepWithCrash(SLEEP_DELAY, timeout, slept);
	}

	// Mailbox is readable
	hardware::mailbox::WRITE[0] = (uint32_t)buffer | 0x08;

	// Wait for the answer to be readable
	slept = 0;
	while(true) {
		while((hardware::mailbox::STATUS[0] & STATUS_EMPTY) == 0) {
			sleepWithCrash(SLEEP_DELAY, timeout, slept);
		}
		uint32_t out = hardware::mailbox::STATUS[0];
		if((out & 0x0F) == 0x08 && // right channel
				(out & 0xFFFFFFF0) == (Ptr)buffer) // right buffer (cf async)
		{
			break;
		}
	}
}

void makeBuffer(uint32_t*& buff, uint32_t*& freePtr, size_t size) {
	uint8_t* mem = (uint8_t*)malloc(size+16);
	buff = (uint32_t*) (mem + ((16 - ((Ptr)mem & 0xFFFFFFF0)) & (0xFFFFFFF0)));
	freePtr = (uint32_t*) mem;
}

// ========= Easy access functions

uint32_t getBoardModel() {
	uint32_t *buff, *freePtr;
	makeBuffer(buff,freePtr, 16*4);
	buildModelRequest(buff);
	readTag(buff, 1000*1000);
	uint32_t out = buff[5];
	free(freePtr);
	return out;
}
	
uint32_t getBoardRevision() {
	uint32_t *buff, *freePtr;
	makeBuffer(buff,freePtr, 16*4);
	buildRevisionRequest(buff);
	readTag(buff, 1000*1000);
	uint32_t out = buff[5];
	free(freePtr);
	return out;
}
	
void getMac(uint8_t* out) {
	uint32_t *buff, *freePtr;
	makeBuffer(buff,freePtr, 16*4);
	buildMacRequest(buff);
	readTag(buff, 1000*1000);
	uint8_t* readPos = (uint8_t*) (buff+5);
	for(int pos=0; pos < 6; pos++)
		out[pos] = readPos[pos];
	free(freePtr);
}
	
uint32_t getRamSize() {
	uint32_t *buff, *freePtr;
	makeBuffer(buff,freePtr, 16*4);
	buildTotalRamRequest(buff);
	readTag(buff, 1000*1000);
	uint32_t out = buff[6];
	free(freePtr);
	return out;
}
	
} // END NAMESPACE

