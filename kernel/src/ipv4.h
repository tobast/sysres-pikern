#pragma once

#include "Bytes.h"

namespace ipv4 {
	Bytes& formatPacket(Bytes& pck, const Bytes& payload, Ipv4Addr toAddr,
			uint8_t protocol);
	/**
	 * Writes [payload] to [pck], decorated with IPv4 headers, to be sent
	 * to [toAddr].
	 **/

	class WrongProtocol {};
	class BadChecksum {};

	struct PckInfos {
		PckInfos() : dataSize(0), fromAddr(0), toAddr(0), protocol(0) {}
		uint16_t dataSize;
		Ipv4Addr fromAddr, toAddr;
		uint8_t protocol;
	};

	PckInfos extractHeader(Bytes& pck);
	/**
	 * Extracts the header from [pck], returning a [PckInfos] structure
	 * containing all the useful data collected from the header.
	 * Throws [WrongProtocol] if the packet is not IPv4 (naive checks).
	 * Throws [BadChecksum] if the Ipv4 header checksum is incorrect.
	 **/
} // END NAMESPACE 
