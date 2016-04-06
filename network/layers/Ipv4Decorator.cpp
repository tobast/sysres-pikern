#include "Ipv4Decorator.h"

Ipv4Decorator::Ipv4Decorator(Ipv4Addr sourceIp) :
	uuid(0), sourceIp(sourceIp) {}

Bytes Ipv4Decorator::decorate(const Bytes& data, const Ipv4Addr destIp) {
	//TODO return more than one packet if needed.
	Bytes out;
	out << (uchar)((0x4 << 4) | (0x5))
		<< (uchar)((0x0 << 2) | 0x0) // DSCP << 2 | ECN
		<< (ushort)(data.size() + 20)
		<< uuid
		<< (ushort)((0b000 << 13) | 0x0)
		<< (uchar)(64)
		<< (uchar)(0x06)
		<< (ushort)(0) // TODO checksum
		<< sourceIp
		<< destIp;
	uuid++;
	return out;
}

size_t Ipv4Decorator::UuidHash::operator()(const Uuid& v) const {
	return std::hash<unsigned short>()((unsigned short)v);
}

