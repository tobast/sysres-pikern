#include "uspi_interface.h"

#include "assert.h"
#include "common.h"
#include "sleep.h"
#include "mailbox.h"
#include "gpio.h"
#include "networkCore.h"
#include "malloc.h"
#include "svc.h"
#include "logger.h"

#include <cstdio>
#include <cstdarg>

void usDelay(unsigned nMicroSeconds) {
	sleep(nMicroSeconds);
}
void MsDelay(unsigned nMilliSeconds) {
	sleep(1000*nMilliSeconds);
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
	sleep(3*stabTime);
	return 1;
}

/// Returns the MAC address of the device.
int GetMACAddress(u8 Buffer[6]) {
	uint64_t mac = mailbox::getMac();
	for(int i=0; i < 6; i++) {
		// FIXME: right byte order?
		Buffer[i] = (u8) mac;
		mac >>= 8;
	}
	return 1; // success
}

void LogWrite(const char* pSource,
		unsigned Severity,
		const char* pMessage,
		...)
{
	va_list args;
	va_start(args, pMessage);

	appendLog(logLevelFromUSPi(Severity), pSource, pMessage, args);

	va_end(args);
}

void uspi_assertion_failed (const char *pExpr, const char *pFile,
		unsigned nLine) {
	gpio::blink(gpio::CRASH_PIN);
	gpio::blink(gpio::CRASH_PIN);
	gpio::blinkValue(nLine);
	LogWrite("USPi", logLevelFromUSPi(LOG_ERROR),
			"%s:%u assert failure %s", pFile, nLine, pExpr);
	assert(false, 0x22);
}

// display hex dump (pSource can be 0)
void DebugHexdump (const void* /*pBuffer*/, unsigned /*nBufLen*/,
		const char* /*pSource*/ /* = 0 */) {
}
