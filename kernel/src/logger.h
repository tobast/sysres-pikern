#pragma once

#include <cstdarg>
#include "Bytes.h"

enum LogLevels {
	LogError, LogWarning, LogInfo, LogDebug, LogOther
};

namespace logger {
	void init();
	void mainLoop();
	void addListener(Ipv4Addr addr);
}

LogLevels logLevelFromUSPi(unsigned logLevel);

void appendLog(const char* fmt, ...);
void appendLog(const char* fmt, va_list args);
void appendLog(LogLevels severity, const char* source, const char* fmt, ...);
void appendLog(LogLevels severity, const char* source, const char* fmt,
		va_list args);
void appendLog(const Bytes& payload);

