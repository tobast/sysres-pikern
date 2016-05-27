#include "assert.h"
#include "gpio.h"
#include "logger.h"

extern "C" {

void __attribute__((noreturn)) _assert_fail(const char* file, const char* func, int line,
		const char* expr, uint8_t assertId)
{
	gpio::dispByte(assertId);
	if(assertId != 0) {
		appendLog(LogError, "assert failure", "in %s:%s:%d, in %s, "
				"with ID %02X.",
				file, func, line, expr, assertId);
	}
	else {
		appendLog(LogError, "assert failure", "in %s:%s:%d, in %s.",
				file, func, line, expr);
	}
	crash();
}

void __attribute__((noreturn)) crash() {
	gpio::set(gpio::ACT_PIN);
	gpio::unset(gpio::LED_PIN);
	gpio::set(gpio::CRASH_PIN);
	while (1) {}
}

}

