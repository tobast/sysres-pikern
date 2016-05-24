#include "assert.h"
#include "gpio.h"
#include "logger.h"

void _assert(bool b, const char* file, const char* func, int line,
		const char* expr, uint8_t assertId)
{
	if (!b) {
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
}

void crash() {
	gpio::set(gpio::ACT_PIN);
	gpio::unset(gpio::LED_PIN);
	gpio::set(gpio::CRASH_PIN);
	while (1) {}
}
