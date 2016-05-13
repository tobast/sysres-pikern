#include <cstdint>
#include <cstring>
#include "udp.h"
#include "mailbox.h"

namespace udp {

const uint16_t ETHERTYPE = 0x0800,
	  ETHER_HEAD_LEN = 14, IPV4_HEAD_LEN = 20, UDP_HEAD_LEN = 8;
const uint16_t HEAD_LEN = ETHER_HEAD_LEN + IPV4_HEAD_LEN + UDP_HEAD_LEN;

unsigned add(uint8_t v, uint8_t* pck, unsigned pos) {
	pck[pos] = v;
	return pos+1;
}
unsigned add(uint16_t v, uint8_t* pck, unsigned pos) {
	pck[pos] = v >> 8;
	pck[pos+1] = v & 0xff;
	return pos+2;
}
unsigned add(uint32_t v, uint8_t* pck, unsigned pos) {
	pck[pos] = v >> 24;
	pck[pos+1] = (v >> 16) & 0xff;
	pck[pos+2] = (v >> 8) & 0xff;
	pck[pos+3] = v & 0xff;
	return pos+4;
}

unsigned formatPacket(void* packet, const void* data, unsigned length,
			uint32_t fromAddr, uint16_t fromPort,
			uint32_t toAddr, uint16_t toPort) {
	static uint16_t packetId = 42;

	unsigned pos = 0;
	uint8_t* out = (uint8_t*)packet;

	// ========== ETHERNET LAYER PROTOCOL =============
	//TODO get real destination MAC address (ARP ?)
	uint64_t distantMac = 0x6c3be58c2917;
	pos = add((uint32_t)(distantMac>>16), out, pos);
	pos = add((uint16_t)distantMac, out, pos);
	uint64_t selfAddr = mailbox::getMac();
	pos = add((uint32_t)(selfAddr>>16), out, pos);
   	pos = add((uint16_t)(selfAddr), out, pos);
	pos = add(ETHERTYPE, out, pos);

	// ========== ETHERNET LAYER PROTOCOL =============
	pos = add((uint8_t) 0x45, out, pos);
   	pos = add((uint8_t) 0x00, out, pos);
	pos = add((uint16_t)(length + IPV4_HEAD_LEN + UDP_HEAD_LEN), out, pos);
	pos = add(packetId, out, pos);
   	pos = add((uint16_t) 0x00, out, pos);
	pos = add((uint8_t) 0xFF, out, pos);
   	pos = add((uint8_t) 0x11, out, pos);
   	pos = add((uint16_t) 0x0000, out, pos);
	pos = add((uint32_t) fromAddr, out, pos);
	pos = add((uint32_t) toAddr, out, pos);

	// Checksum
	uint32_t chksum = 0;
	for(unsigned cpos=ETHER_HEAD_LEN; cpos < pos; cpos+=2) {
		chksum += (((uint16_t)out[cpos])<<8)+(out[cpos+1]);
		chksum = (chksum & 0xffff) + (chksum >> 16);
	}
	uint16_t fChksum = ~((uint16_t)chksum);
	out[ETHER_HEAD_LEN+10] = fChksum >> 8;
	out[ETHER_HEAD_LEN+11] = fChksum;

	// ========== UDP LAYER PROTOCOL =============
	pos = add(fromPort, out, pos);
   	pos = add(toPort, out, pos);
	pos = add((uint16_t)(UDP_HEAD_LEN + length), out, pos);
   	pos = add((uint16_t) 0x00, out, pos);

	// ========== DATA ===========================
	memcpy(out+pos, data, length);

	packetId += 7;

	return pos + length;
}

} //END NAMESPACE
