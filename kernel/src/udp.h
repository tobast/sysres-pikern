#pragma once

#include "networkCore.h"
#include "Bytes.h"

namespace udp {
	/*
	unsigned formatPacket(void* packet, const void* data, unsigned length,
			uint32_t fromAddr, uint16_t fromPort,
			uint32_t toAddr, uint16_t toPort); */
	/** Writes an UTF-8 formatted packet in [packet], containing the
	 * first [length] bytes of [data] as its data. Returns the size of
	 * [packet] after being written.
	 **/

	Bytes& formatPacket(Bytes& pck, const Bytes& data,
			uint16_t fromPort,
			Ipv4Addr toAddr, uint16_t toPort);
	/** Writes [data] to [pck], decorated with IPv4 and UDP headers,
	 * sending to [toAddr]:[toPort] from [fromPort].
	 **/

	class WrongProtocol {};
	class BadChecksum {};

	struct PckInfos {
		unsigned dataSize;
		Ipv4Addr fromAddr, toAddr;
		uint16_t fromPort, toPort;
	};

	PckInfos extractHeader(Bytes& pck);
	/** Reads the IPv4 + UDP headers, extracts them from [pck] and returns
	 * the extracted informations in a PckInfos structure.
	 * Throws [WrongProtocol] if [pck] is not UDP,
	 * [BadChecksum] if the IPv4 header checksum is incorrect.
	 **/
}
