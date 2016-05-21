#include "logger.h"
#include "uspi_interface.h"
#include "format.h"
#include "interrupts.h"
#include "networkCore.h"

const char* stringOfLogLevel(LogLevels severity) {
	switch(severity) {
		case LogError:	return "ERROR";
		case LogWarning: return "WARNING";
		case LogInfo: return "INFO";
		case LogDebug: return "DEBUG";
		case LogOther: return "DAFUQ";
	}
	return "DAFUQ";
}

LogLevels logLevelFromUSPi(unsigned logLevel) {
	switch(logLevel) {
		case LOG_ERROR: return LogError;
		case LOG_WARNING: return LogWarning;
		case LOG_NOTICE: return LogInfo;
		case LOG_DEBUG: return LogDebug;
	}
	return LogOther;
}

void appendLog(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	appendLog(fmt,args);
	
	va_end(args);
}

void appendLog(const char* fmt, va_list args) {
	Bytes payload;
	formatToBytes(payload, fmt, args);
	payload << '\n';

	appendLog(payload);
}

void appendLog(LogLevels severity, const char* source, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	appendLog(severity, source, fmt, args);
	va_end(args);
}
void appendLog(LogLevels severity, const char* source, const char* fmt,
		va_list args)
{
	Bytes payload;
	payload << '[' << stringOfLogLevel(severity) << "] "
		<< source << ": ";
	formatToBytes(payload, fmt, args);
	payload << '\n';

	appendLog(payload);
}

void appendLog(const Bytes& payload) {
	if(is_interrupt()) // endangers mutexes.
		return;

	Bytes udpPacket;
	udp::formatPacket(udpPacket, payload, 1, nw::DEST_IP, 3141);

	nw::sendPacket(udpPacket, nw::DEST_IP);
}

