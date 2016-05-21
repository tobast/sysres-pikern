#include "logger.h"
#include "uspi_interface.h"
#include <cstdio>

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
	Bytes payload;
	char* buff = (char*)malloc(256);
	vsprintf(buff, fmt, args);
	payload << "[" << stringOfLogLevel(severity) << "] "
		<< source << ": " << buff << "\n";
	//formatToBytes(payload, fmt, args);

/*
	nw::logAppend(source);
	nw::logAppend(" [");
	nw::logAppend(stringOfLogLevel(severity));
	nw::logAppend("] ");
	nw::logAppend(fmt);
	nw::logAppend("\n");
//	nw::logAppend(payload);
//	gpio::blink(gpio::LED_PIN);
*/

//	nw::logAppend("OHai!\n");
	nw::logAppend(payload);	
}

