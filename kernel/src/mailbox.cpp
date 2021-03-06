#include "mailbox.h"
#include "barriers.h"
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
const uint32_t VAL_MODEL =		0x00010001;
const uint32_t VAL_REV =		0x00010002;
const uint32_t VAL_MACADDR =	0x00010003;
const uint32_t VAL_RAMSIZE =	0x00010005;
const uint32_t GET_POWSTATE =	0x00020001;
const uint32_t GET_POWTIMING =	0x00020002;
const uint32_t SET_POWSTATE =	0x00028001;
const uint32_t GET_TEMP =		0x00030006;
const uint32_t GET_CRIT_TEMP =	0x0003000A;
// ===== END MAILBOX CONSTANTS ===========

typedef uint32_t Ptr;

void checkAlignment(uint32_t volatile* buffer) {
	if(((Ptr)buffer & 0xFFFFFFF0) != (Ptr)buffer)
		throw new WrongAlignment;
}

uint32_t* initBuffer(uint32_t volatile* buffer) {
	/// Returns the next position to write, as an uint32_t*
	checkAlignment(buffer);
	buffer[0] = 0x00000000;
	buffer[1] = 0x00000000;
	return (uint32_t*)(buffer+2);
}

/// Closes the tag [buffer], which next free-to-write position is [pos].
void closeBuffer(uint32_t volatile* buffer, uint32_t* pos) {
	// End tag (0x00000000)
	pos[0] = 0x00;
	pos++;

	// Tag size
	buffer[0] = (uint32_t) ((Ptr)pos - (Ptr)buffer);
}

uint32_t* writeTag(uint32_t volatile* buffer, uint32_t tagId,
		size_t bufLen, size_t valLen, uint32_t** bufPos = NULL);
uint32_t* writeTag(uint32_t volatile* buffer, uint32_t tagId,
		size_t bufLen, size_t valLen, uint32_t** bufPos)
	/// Returns the next writeable position. If bufPos != NULL, sets
	/// *bufPos to the address where the tag's values can be written.
{
	assert(bufLen % 4 == 0, 0x88); // 32b-padded

	buffer[0] = tagId;
	buffer[1] = bufLen;
	buffer[2] = 0x7fffffff & valLen;

	for(size_t pos=3; pos < bufLen+3; pos++)
		buffer[pos] = 0x00;

	if(bufPos != NULL)
		*bufPos = (uint32_t*)buffer+3;
	
	return (uint32_t*) (buffer + 3 + bufLen/4);
}

void buildMacRequest(uint32_t volatile* buffer) {
	uint32_t* writeAddr = initBuffer(buffer);
	writeAddr = writeTag(writeAddr, VAL_MACADDR, 8, 0);
	closeBuffer(buffer, writeAddr);
}
void buildModelRequest(uint32_t volatile* buffer) {
	uint32_t* writeAddr = initBuffer(buffer);
	writeAddr = writeTag(writeAddr, VAL_MODEL, 4, 0);
	closeBuffer(buffer, writeAddr);
}
void buildRevisionRequest(uint32_t volatile* buffer) {
	uint32_t* writeAddr = initBuffer(buffer);
	writeAddr = writeTag(writeAddr, VAL_REV, 4, 0);
	closeBuffer(buffer, writeAddr);
}
void buildTotalRamRequest(uint32_t volatile* buffer) {
	uint32_t* writeAddr = initBuffer(buffer);
	writeAddr = writeTag(writeAddr, VAL_RAMSIZE, 8, 0);
	closeBuffer(buffer, writeAddr);
}

void readTag(uint32_t volatile* buffer, uint32_t timeout) {
	checkAlignment(buffer); // If the alignment is wrong, we could hang forever

	uint32_t start_us = elapsed_us();
	uint32_t out;
	// Wait for the buffer to be writeable
	while(hardware::mailbox::STATUS[0] & STATUS_FULL) {
		flushcache();
		assert(timeout == 0 || (((u32)elapsed_us()) - start_us) < timeout,
				0x02);
	}

	// Write the request
	dataMemoryBarrier();
	hardware::mailbox::WRITE[0] = (uint32_t)buffer | 0x08;
	dataMemoryBarrier();

	// Wait for the answer to be readable
	// NOTE: shall we reset start_us here? Do we want a global timeout, or a
	// 		timeout per mailbox interaction?
	while(true) {
		while(hardware::mailbox::STATUS[0] & STATUS_EMPTY) {
			flushcache();
			assert(timeout == 0 || (((u32)elapsed_us()) - start_us) < timeout,
					0x03);
		}
		dataMemoryBarrier();
		out = hardware::mailbox::READ[0];
		dataMemoryBarrier();
		if((out & 0x0F) == 0x08 && // right channel
				(out & 0xFFFFFFF0) == (Ptr)buffer) // right buffer (cf async)
		{
			break;
		}
	}
	// TODO maybe check result code?
}

void makeBuffer(uint32_t volatile*& buff, uint32_t*& freePtr,
		size_t size) {
	uint8_t* mem = (uint8_t*)malloc(size+16);
	buff = (uint32_t*) (mem + ((16 - ((Ptr)mem & 0xF)) & (0xFFFFFFF0)));
	freePtr = (uint32_t*) mem;
}

// ========= Easy access functions

uint32_t getBoardModel() {
	uint32_t volatile *buff;
	uint32_t *freePtr;
	makeBuffer(buff,freePtr, 16*4);
	buildModelRequest(buff);
	readTag(buff, 1000*1000);
	uint32_t out = buff[5];
	free(freePtr);
	return out;
}

uint32_t getBoardRevision() {
	uint32_t volatile *buff;
	uint32_t *freePtr;
	makeBuffer(buff,freePtr, 16*4);
	buildRevisionRequest(buff);
	readTag(buff, 1000*1000);
	uint32_t out = buff[5];
	free(freePtr);
	return out;
}

uint64_t getMac() {
	static uint64_t cached = 0x0; // Cache the result for future use.
	
	if(cached != 0)
		return cached;

	uint32_t volatile *buff;
	uint32_t *freePtr;
	makeBuffer(buff,freePtr, 16*4);
	buildMacRequest(buff);
	readTag(buff, 1000*1000);
	uint64_t out = buff[5] | (((uint64_t)buff[6]) << 32);
	cached = out;
	free(freePtr);
	return out;
}
	
uint32_t getRamSize() {
	uint32_t volatile *buff;
	uint32_t *freePtr;
	makeBuffer(buff,freePtr, 16*4);
	buildTotalRamRequest(buff);
	readTag(buff, 1000*1000);
	uint32_t out = buff[6];
	free(freePtr);
	return out;
}

uint32_t getPowerState(uint32_t deviceId) {
	uint32_t volatile *buff;
	uint32_t *freePtr, *argsPos;

	makeBuffer(buff, freePtr, 16*4);
	uint32_t* writePos = initBuffer(buff);
	writePos = writeTag(writePos, GET_POWSTATE, 8, 4, &argsPos);
	argsPos[0] = deviceId;
	closeBuffer(buff, writePos);

	readTag(buff, 1000*1000);

	uint32_t out = buff[6];
	free(freePtr);
	return out;
}
	
uint32_t getPowerupTiming(uint32_t deviceId) {
	uint32_t volatile *buff;
	uint32_t *freePtr, *argsPtr;
	makeBuffer(buff, freePtr, 16*4);
	uint32_t* writePos = initBuffer(buff);
	writePos = writeTag(writePos, GET_POWTIMING, 8, 4, &argsPtr);
	argsPtr[0] = deviceId;
	closeBuffer(buff, writePos);

	readTag(buff, 1000*1000);
	uint32_t out = buff[6];
	free(freePtr);
	return out;
}
	
uint32_t setPowerState(uint32_t deviceId, uint32_t powerStatus) {
	uint32_t volatile *buff;
	uint32_t *freePtr, *argsPtr;
	makeBuffer(buff, freePtr, 16*4);
	uint32_t* writePos = initBuffer(buff);
	writePos = writeTag(writePos, SET_POWSTATE, 8, 8, &argsPtr);
	argsPtr[0] = deviceId;
	argsPtr[1] = powerStatus;
	closeBuffer(buff, writePos);

	readTag(buff, 1000*1000);
	uint32_t out = buff[6];
	free(freePtr);
	return out;
}

uint32_t getCpuTemp() {
	uint32_t volatile *buff;
	uint32_t *freePtr, *argsPtr;
	makeBuffer(buff, freePtr, 16*4);
	uint32_t* writePos = initBuffer(buff);
	writePos = writeTag(writePos, GET_TEMP, 8, 4, &argsPtr);
	argsPtr[0] = 0x00;
	closeBuffer(buff, writePos);

	readTag(buff, 1000*1000);
	uint32_t out = buff[6];
	free(freePtr);
	return out;
}

uint32_t getCriticalCpuTemp() {
	uint32_t volatile *buff;
	uint32_t *freePtr, *argsPtr;
	makeBuffer(buff, freePtr, 16*4);
	uint32_t* writePos = initBuffer(buff);
	writePos = writeTag(writePos, GET_CRIT_TEMP, 8, 4, &argsPtr);
	argsPtr[0] = 0x00;
	closeBuffer(buff, writePos);
	
	readTag(buff, 1000*1000);
	uint32_t out = buff[6];
	free(freePtr);
	return out;
}
	
} // END NAMESPACE

