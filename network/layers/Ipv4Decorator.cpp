#include "Ipv4Decorator.h"

namespace network {

Ipv4Decorator::Ipv4Decorator(Ipv4Addr sourceIp) :
	uuid(0), sourceIp(sourceIp) {}

Bytes Ipv4Decorator::decorateSingle(const Packet& packet,
		bool notLastFragment) {
	if(packet.data.size() > maxPacketLoad())
		throw (new TooLargePacket);

	Bytes out;
	out << (uchar)((0x4 << 4) | (0x5))
		<< (uchar)((0x0 << 2) | 0x0) // DSCP << 2 | ECN
		<< (uint16_t)(packet.data.size() + 20)
		<< uuid
		<< (uint16_t)((0b00 << 14) | (notLastFragment << 13) | 0x0)
		<< (uchar)(64)
		<< packet.proto
		<< (uint16_t)(0)
		<< sourceIp
		<< packet.addr;
	uuid++;
	
	uint16_t chksum = checksum(out);
	out[10] = chksum>>8;
	out[11] = chksum % 0xFF;

	out << packet.data;

	return out;
}

std::vector<Bytes> Ipv4Decorator::decorate(const Packet& packet){
	std::vector<Bytes> out;
	
	for(size_t pos = 0; pos < packet.data.size(); pos += maxPacketLoad()) {
		size_t subLen = maxPacketLoad();
		bool isLast = false;
		if(pos + maxPacketLoad() >= packet.data.size()) {
			isLast = true;
			subLen = packet.data.size() - pos;
		}
		
		Packet subpack;
		subpack.data = packet.data.sub(pos,subLen);
		subpack.addr = packet.addr;
		subpack.proto = packet.proto;
		out.push_back(decorateSingle(subpack,isLast));
	}
	return out;
}

Ipv4Decorator::Packet Ipv4Decorator::extract(const Bytes& packet) {
	char version=packet.at(0);
	char ihl=0;
	ihl = version & 0xF;
	version >>= 4;

	if(version != 4)
		throw (new InvalidPacket);

	Bytes head = packet.sub(1, ihl-1); // we don't want the first byte
	Bytes body = packet.sub(ihl, packet.size() - ihl);

	// Extract packet
	char ignore,flags,ttl,proto;
	uint16_t fullSize,packUuid,offset,chksum;
	uint32_t source,dest;
	head >> ignore
		>> fullSize
		>> packUuid
		>> offset
		>> ttl
		>> proto
		>> chksum
		>> source
		>> dest;

	flags = offset >> 3;
	offset &= 0x1FFF;
	
	// Check packet size
	if(fullSize != packet.size())
		throw (new InvalidPacket);

	// Check the checksum
	if(checksum(packet.sub(0,ihl)) != chksum)
		throw (new InvalidPacket);
	
	// Check we were meant to receive that
	if(dest != sourceIp)
		throw (new InvalidPacket);

	// Check whether the packet is a datagram fragment
	Packet out;
	if(flags & (1<<0) || offset > 0) {
//		FragmentIdentifier fragId(source, dest, packUuid, proto);
//		DatagramFragment& curFrag = uncompleted[fragId];
		
		//TODO
		throw (new IncompletePacket); // NOT IMPLEMENTED.

		//if(curFrag.frags.empty()) { // First fragment received
	}
	else { // Not fragmented
		out.data = body;
		out.addr = source;
		out.proto = proto;
	}
	return out;
}

size_t Ipv4Decorator::maxPacketLoad() {
	return (1<<16) - 20;
}

uint16_t Ipv4Decorator::checksum(const Bytes& header) const {
	/// Computes the checksum of [header]. Does *not* check the given
	/// header's length!
	
	uint32_t out=0;
	for(size_t shPos=0; shPos < header.size() / 16; shPos++) {
		if(shPos == 5) // Don't sum up the checksum short.
			continue;
		out += header.ushortAt(shPos);
		out = (out % 0xFFFF) + (out >> 16);
	}
	return ~((uint16_t)out);
}

size_t Ipv4Decorator::FragmentIdentifierHash::operator()
	(const FragmentIdentifier& v) const
{
	return (size_t)(
			std::hash<unsigned int>()(v.from) +
			std::hash<unsigned int>()(v.to) +
			std::hash<unsigned short>()(v.uuid) +
			std::hash<unsigned char>()(v.protocol));
}

} // END NAMESPACE

