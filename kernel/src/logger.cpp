#include "logger.h"
#include "uspi_interface.h"
#include "format.h"
#include "interrupts.h"

const char* stringOfLogLevel(unsigned severity) {
	switch(severity) {
		case LOG_ERROR:	return "ERROR";
		case LOG_WARNING: return "WARNING";
		case LOG_NOTICE: return "INFO";
		case LOG_DEBUG: return "DEBUG";
	}
	return "DAFUQ";
}

void appendLog(const char* source, unsigned severity, const char* fmt,
		va_list args)
{
	if(is_interrupt()) // Endangers mutexes.
		return;
	Bytes payload;
	payload << '[' << stringOfLogLevel(severity) << "] "
		<< source << ": ";
	formatToBytes(payload, fmt, args);
	payload << '\n';

	nw::logAppend(payload);	
}

