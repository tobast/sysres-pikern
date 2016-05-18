#include "arp.h"

namespace arp {

const uint16_t HTYPE=1,
	  PTYPE=0x0800,
	  OPER_REQ=1, OPER_REPL=2;
const uint8_t HLEN=6,
	  PLEN=4;

// ARP cache: keeps record of every previously-seen MAC-IPv4 match.
//std::map<Ipv4Addr, HwAddr> arpCache;

Bytes& formatHeaderBeg(Bytes& buffer) {
	buffer << HTYPE << PTYPE << HLEN << PLEN;
	return buffer;
}

Bytes& formatQuery(Bytes& buffer, Ipv4Addr addr) {
	nw::fillEthernetHeader(buffer, (HwAddr)0xFFFFFFFFFFFF);
	formatHeaderBeg(buffer);
	buffer << OPER_REQ;
	buffer.appendHw(nw::getHwAddr());
	buffer << nw::getEthAddr();
	buffer.appendHw((HwAddr)0xFFFFFFFFFFFF);
	buffer << addr;
	return buffer;
}

Bytes& formatReply(Bytes& buffer, HwAddr macTo, Ipv4Addr ipTo) {
	nw::fillEthernetHeader(buffer, macTo);
	formatHeaderBeg(buffer);
	buffer << OPER_REPL;
	buffer.appendHw(nw::getHwAddr());
	buffer << nw::getEthAddr();
	buffer.appendHw(macTo);
	buffer << ipTo;
	return buffer;
}

} // END NAMESPACE
