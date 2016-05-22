#include "ipv4.h"

#include "common.h"
#include "networkCore.h"
#include "malloc.h"

namespace ipv4 {

const uint16_t IPV4_HEAD_LEN = 20;

uint16_t ipv4Chksum(const Bytes& b, unsigned headBeg, unsigned headEnd,
		uint16_t firstShort = 0) {
	uint32_t chksum = firstShort;
	for(unsigned cpos=headBeg; cpos < headEnd; cpos+=2) {
		chksum += (((uint16_t)b.at(cpos)<<8)+(b.at(cpos+1)));
		chksum = (chksum & 0xffff) + (chksum >> 16);
	}
	return ~chksum;
}


Bytes& formatPacket(Bytes& pck, const Bytes& payload, Ipv4Addr toAddr) {
	static uint16_t packetId = 42;

	unsigned ipv4BegPos = pck.size();

	// IPv4 header
	pck << (uint8_t) 0x45 << (uint8_t) 0x00
		<< (uint16_t) (payload.size() + IPV4_HEAD_LEN)
		<< packetId << (uint16_t) 0x00 << (uint8_t) 0xff
		<< (uint8_t) 0x11 << (uint16_t) 0x00 << nw::getEthAddr() << toAddr;
	// IPv4 checksum
	uint16_t chksum = ipv4Chksum(pck, ipv4BegPos, pck.size());
	pck[ipv4BegPos + 10] = (chksum >> 8);
	pck[ipv4BegPos + 11] = (chksum & 0xFF);

	// Payload
	pck << payload;

	// "unique" packet ID :°
	packetId += 7;
	return pck;
}

PckInfos extractHeader(Bytes& pck) {
	PckInfos infos;

	uint8_t ignoreU8;
	uint16_t ignoreU16;
	uint32_t ignoreU32;

	uint8_t firstByte;
	unsigned afterLength;

	// Read protocol nibble and header length nibble
	pck >> firstByte >> ignoreU8;
	if((firstByte >> 4) != 0x4 || (firstByte & 0xf) < 5) // Not IPv4
		throw WrongProtocol();
	afterLength = (firstByte & 0xf) - 5;

	// Check checksum
	uint16_t chksumFirstShort = (((uint16_t)firstByte)<<8) + ignoreU8;
	if(ipv4Chksum(pck, 0, IPV4_HEAD_LEN+4*afterLength-2, chksumFirstShort)
			!= 0x00)
		throw BadChecksum();

	// Packet size
	pck >> infos.dataSize;
	infos.dataSize -= IPV4_HEAD_LEN+4*afterLength;

	// Protocol
	pck >> ignoreU32 >> ignoreU8 >> infos.protocol;

	// Addresses
	pck >> ignoreU16 >> infos.fromAddr >> infos.toAddr;

	// Header end
	for(unsigned l=0; l < afterLength; l++)
		pck >> ignoreU32;

	return infos;
}

} // END NAMESPACE
