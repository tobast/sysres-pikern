#pragma once

#include "networkCore.h"
#include "Bytes.h"
#include "ipv4.h"

namespace udp {
	const uint8_t PROTOCOL_ID = 0x11;
	Bytes& formatPacket(Bytes& pck, const Bytes& data,
			uint16_t fromPort,
			Ipv4Addr toAddr, uint16_t toPort);
	/** Writes [data] to [pck], decorated with IPv4 and UDP headers,
	 * sending to [toAddr]:[toPort] from [fromPort].
	 **/

	Bytes& formatUdpHeader(Bytes& pck, const Bytes& data,
			uint16_t fromPort, uint16_t toPort);
	/**
	 * Writes [data] to [pck], decorated with UDP header only, sending to
	 * ?:[toPort] from [fromPort].
	 **/

	class WrongProtocol {};

	struct PckInfos {
		PckInfos() {}
		PckInfos(const ipv4::PckInfos& p);
		uint16_t dataSize;
		Ipv4Addr fromAddr, toAddr;
		uint16_t fromPort, toPort;
	};

	PckInfos extractHeader(Bytes& pck);
	/** Reads the IPv4 + UDP headers, extracts them from [pck] and returns
	 * the extracted informations in a PckInfos structure.
	 * Throws [WrongProtocol] if [pck] is not UDP.
	 * Throws [ipv4::WrongProtocol] if [pck] is not IPv4.
	 * Throws [ipv4::BadChecksum] if the IPv4 header checksum is incorrect.
	 **/

	PckInfos extractUdpHeader(Bytes& pck, PckInfos infos=PckInfos());
	/**
	 * Reads the UDP header only, extract it from [pck] and returns the
	 * extracted informations in a PckInfos structure.
	 * Throws [WrongProtocol] if [pck] is not UDP.
	 **/
}

