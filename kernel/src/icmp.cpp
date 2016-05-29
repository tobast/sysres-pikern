#include "icmp.h"

namespace icmp {
bool respondToPing(Bytes& pck, const ipv4::PckInfos& infos) {
	unsigned pckSize = infos.dataSize;
	if(pck.size() < 8 || pckSize < 8) // min ICMP header
		return false;

	Bytes resp;
	resp << (uint32_t) 0x00;

	if(pckSize % 2 == 0) { // What are we supposed to do with odd parity?
		if(nw::networkChksum(pck, 0, pckSize) != 0x00)
			throw BadChecksum();
	}

	uint8_t type, code, payloadByte;
	uint16_t ignoreU16, idNum, seqNum;

	pck >> type >> code >> ignoreU16 >> idNum >> seqNum;
	if(type != 0x8 || code != 0x0) // Not ICMP echo request
		return false;

	// Seems to be a ping request
	resp << idNum << seqNum;
	for(unsigned b=8; b < pckSize; b++) { //copying payload
		pck >> payloadByte;
		resp << payloadByte;
	}

	// Send the packet
	Bytes outPck;
	ipv4::formatPacket(outPck, resp, infos.fromAddr, PROTOCOL_ID);
	nw::sendPacket(outPck, infos.fromAddr);
	
	return true;
}

} // END NAMESPACE

