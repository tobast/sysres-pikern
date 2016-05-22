#include <cstdint>
#include <cstring>
#include "udp.h"
#include "mailbox.h"

namespace udp {

const uint16_t UDP_HEAD_LEN = 8;

PckInfos::PckInfos(const ipv4::PckInfos& p) :
		dataSize(p.dataSize - UDP_HEAD_LEN),
		fromAddr(p.fromAddr), toAddr(p.toAddr)
{
	if(p.protocol != 0x11)
		throw WrongProtocol();
}

Bytes& formatPacket(Bytes& pck, const Bytes& data,
		uint16_t fromPort,
		Ipv4Addr toAddr, uint16_t toPort)
{
	Bytes udpFormatted;
	formatUdpHeader(udpFormatted, data, fromPort, toPort);

	ipv4::formatPacket(pck, udpFormatted, toAddr);
	return pck;
}

Bytes& formatUdpHeader(Bytes& pck, const Bytes& data,
		uint16_t fromPort, uint16_t toPort)
{
	pck << fromPort << toPort
		<< (uint16_t)(UDP_HEAD_LEN + data.size()) << (uint16_t) 0x00
		<< data;

	return pck;
}

PckInfos extractHeader(Bytes& pck) {
	// ==== IPV4 HEADER ====
	ipv4::PckInfos ipv4Infos = ipv4::extractHeader(pck);
	return extractUdpHeader(pck, PckInfos(ipv4Infos));
	return PckInfos();
}

PckInfos extractUdpHeader(Bytes& pck, PckInfos infos) {
	uint16_t ignoreU16;
	pck >> infos.fromPort >> infos.toPort >> infos.dataSize >> ignoreU16;
	infos.dataSize -= UDP_HEAD_LEN;

	return infos;
}

} //END NAMESPACE
