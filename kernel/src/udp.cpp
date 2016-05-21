#include <cstdint>
#include <cstring>
#include "udp.h"
#include "mailbox.h"

namespace udp {

const uint16_t IPV4_HEAD_LEN = 20, UDP_HEAD_LEN = 8;

uint16_t ipv4Chksum(const Bytes& b, unsigned headBeg, unsigned headEnd) {
	uint32_t chksum = 0;
	for(unsigned cpos=headBeg; cpos < headEnd; cpos+=2) {
		chksum += (((uint16_t)b.at(cpos)<<8)+(b.at(cpos+1)));
		chksum = (chksum & 0xffff) + (chksum >> 16);
	}
	return ~chksum;
}

Bytes& formatPacket(Bytes& pck, const Bytes& data,
		uint16_t fromPort,
		Ipv4Addr toAddr, uint16_t toPort)
{
	static uint16_t packetId = 42;

	unsigned ipv4BegPos = pck.size();

	// ==== IPv4 head ====
	pck << (uint8_t) 0x45 << (uint8_t) 0x00
		<< (uint16_t) (data.size() + IPV4_HEAD_LEN + UDP_HEAD_LEN)
		<< packetId << (uint16_t) 0x00 << (uint8_t) 0xff
		<< (uint8_t) 0x11 << (uint16_t) 0x00 << nw::getEthAddr() << toAddr;
	// IPv4 checksum
	uint16_t chksum = ipv4Chksum(pck, ipv4BegPos, pck.size());
	pck[ipv4BegPos + 10] = (chksum >> 8);
	pck[ipv4BegPos + 11] = (chksum & 0xFF);

	// ==== UDP head ====
	pck << fromPort << toPort
		<< (uint16_t)(UDP_HEAD_LEN + data.size()) << (uint16_t) 0x00
		<< data;

	packetId += 7;

	return pck;
}

PckInfos extractHeader(Bytes& pck) {
	PckInfos out;
	uint8_t b1;
	uint16_t chksum;
	uint32_t addr;

	// ==== IPV4 HEADER ====
	chksum = ipv4Chksum(pck, 0, 20);
	if(chksum != 0)
		throw BadChecksum();
	
	pck >> b1;
	if((b1 >> 4) != 4) // not IPv4
		throw WrongProtocol();

	pck >> b1 >> b1 >> b1; // Ignore 3 bytes
	pck >> addr; // Ignore 32 bits
	pck >> b1; // Ignore TTL
	pck >> b1; // B1 = protocol
	if(b1 != 0x11) // Not UDP
		throw WrongProtocol();
	pck >> chksum; // Ignore chksum

	pck >> out.fromAddr >> out.toAddr;

	// ==== UDP HEADER ====
	uint16_t size;
	pck >> out.fromPort >> out.toPort >> size >> chksum;
	out.dataSize = size - 8;

	return out;
}

} //END NAMESPACE
