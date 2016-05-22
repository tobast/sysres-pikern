#pragma once

#include "networkCore.h"
#include "ipv4.h"
#include "Bytes.h"

namespace icmp {
	const uint8_t PROTOCOL_ID = 0x01;
	class BadChecksum {};

	bool respondToPing(Bytes& pck, const ipv4::PckInfos& infos);
	/**
	 * Responds to ping if the given packet was indeed a ping request.
	 * Takes [pck], a network packet extracted from its layer 3 header.
	 * Returns whether the packet was indeed a ping request.
	 * Throws icmp::BadChecksum if the ICMP checksum is wrong.
	 **/
}

