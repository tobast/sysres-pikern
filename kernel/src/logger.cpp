#include "logger.h"
#include "uspi_interface.h"
#include "format.h"
#include "interrupts.h"
#include "networkCore.h"
#include "queue.h"
#include "atomic.h"

Queue<Bytes>* logQueue=NULL;
ExpArray<Ipv4Addr> logDest;
mutex_t* logQueueMutex;
mutex_t* logDestMutex;

namespace logger {
	void init() {
		logQueue = (Queue<Bytes>*)malloc(sizeof(Queue<Bytes>));
		*logQueue = Queue<Bytes>();

		logDest.init();

		logDestMutex = (mutex_t*) malloc(sizeof(mutex_t));
		mutex_init(logDestMutex);

		logQueueMutex = (mutex_t*) malloc(sizeof(mutex_t));
		mutex_init(logQueueMutex);
	}
}

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

void enqueueLog(const Bytes& log) {
	assert(logQueue != NULL, 0xba);
	if(is_interrupt())
		return;
	mutex_lock(logQueueMutex);
	logQueue->push(log);
	mutex_unlock(logQueueMutex);
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
	enqueueLog(payload);
}

void sendLog(const Bytes& log) {
	for(unsigned dest = 0; dest < logDest.size(); dest++) {
		Ipv4Addr destAddr = logDest[dest];

		Bytes udpPacket;
		udp::formatPacket(udpPacket, log, 1, destAddr, 3141);
		nw::sendPacket(udpPacket, destAddr);
	}
}

namespace logger {
	void mainLoop() {
		assert(logQueue != NULL, 0xba);

		while(true) {
			mutex_lock(logDestMutex);
			if(logDest.size() > 0) {
				mutex_lock(logQueueMutex);
				while(logQueue->size() > 0) {
					const Bytes& log = logQueue->pop();
					mutex_unlock(logQueueMutex);
					sendLog(log);
					mutex_lock(logQueueMutex);
				}
				mutex_unlock(logQueueMutex);
			}
			mutex_unlock(logDestMutex);
			sleep(100);
		}
	}

	void addListener(Ipv4Addr addr) {
		mutex_lock(logDestMutex);
		for(unsigned cDest=0; cDest < logDest.size(); cDest++) {
			if(logDest[cDest] == addr)
				return;
		}
		logDest.push_back(addr);
		mutex_unlock(logDestMutex);
	}
}

