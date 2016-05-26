#pragma once

#include "common.h"

struct UdpSysData {
	UdpSysData() {}
	UdpSysData(Ipv4Addr toAddr, uint16_t fromPort, uint16_t toPort,
			void const *data, unsigned len) :
		toAddr(toAddr), fromPort(fromPort), toPort(toPort),
		data(data), len(len)
		{}

	Ipv4Addr toAddr;
	uint16_t fromPort, toPort;
	const void* data;
	unsigned len;
};

