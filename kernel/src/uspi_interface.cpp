#include "uspi_interface.h"

#include "assert.h"
#include "common.h"
#include "sleep.h"
#include "mailbox.h"

void usDelay(unsigned nMicroSeconds) {
	sleep_us(nMicroSeconds);
}
void MsDelay(unsigned nMilliSeconds) {
	sleep_us(1000*nMilliSeconds);
}

/// Powers on the given device, and wait until completed.
int SetPowerStateOn(unsigned nDeviceId) {
	uint32_t stabTime = mailbox::getPowerupTiming(nDeviceId);
	if(stabTime == 0) { // Device does not exists
		return 0;
	}
	
	uint32_t curState = mailbox::getPowerState(nDeviceId);
	if(curState & 0x01) { // Already powered up
		return 1;
	}

	uint32_t nState = mailbox::setPowerState(nDeviceId, 0x11);
	if(!(nState & 0x01)) { // Not powered up
		return 0;
	}
	sleep_us(3*stabTime);
	return 1;
}

/// Returns the MAC address of the device.
int GetMacAddress(u8 Buffer[6]) {
	uint64_t mac = mailbox::getMac();
	for(int i=0; i < 6; i++) {
		// FIXME: right byte order?
		Buffer[i] = (u8) mac;
		mac >>= 8;
	}
	return 1; // success
}

void LogWrite(const char* /*pSource*/,
		unsigned /*Severity*/,
		const char* /*pMessage*/,
		...)
{
	// TODO when we'll be able to dump it somewhere.
}

void uspi_assertion_failed (const char *pExpr, const char *pFile,
		unsigned nLine) {
	assert(false, 19);
}

// display hex dump (pSource can be 0)
void DebugHexdump (const void *pBuffer, unsigned nBufLen,
		const char *pSource /* = 0 */) {
	// TODO
}
