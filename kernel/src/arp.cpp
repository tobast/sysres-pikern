#include "arp.h"
#include "hashTable.h"

namespace arp {

const uint16_t HTYPE=1,
	  PTYPE=0x0800,
	  OPER_REQ=1, OPER_REPL=2;
const uint8_t HLEN=6,
	  PLEN=4;

// ARP cache: keeps record of every previously-seen MAC-IPv4 match.
HashTable<Ipv4Addr, HwAddr> arpCache;

HwAddr cachedHwAddr(const Ipv4Addr& addr) {
	try {
		return arpCache.find(addr);
	} catch(HashTable<Ipv4Addr,HwAddr>::NotFound) {
		return 0;
	}
}

int queryArp(const Ipv4Addr& addr) {
	Bytes pck;
	formatQuery(pck, addr);
	return nw::sendFrame(pck);
}

Bytes& formatHeaderBeg(Bytes& buffer) {
	buffer << HTYPE << PTYPE << HLEN << PLEN;
	return buffer;
}

void readArp(Bytes arp) {
	uint16_t htype,ptype,oper;
	uint8_t hlen,plen;
	HwAddr hwFrom,hwTo;
	Ipv4Addr ipFrom,ipTo;

	arp >> htype >> ptype >> hlen >> plen >> oper;
	if(htype != HTYPE || ptype != PTYPE || hlen != HLEN || plen != PLEN)
		return; // Odd type, ignoring.

	arp.extractHw(hwFrom);
	arp >> ipFrom;
	arp.extractHw(hwTo);
	arp >> ipTo;

	arpCache.insert(ipFrom,hwFrom);
	if(oper == OPER_REQ && ipTo == nw::getEthAddr()) {
		Bytes repl;
		formatReply(repl, hwFrom, ipFrom);
		nw::sendFrame(repl);
	}
}

Bytes& formatQuery(Bytes& buffer, Ipv4Addr addr) {
	nw::fillEthernetHeader(buffer, (HwAddr)0xFFFFFFFFFFFF, nw::ETHERTYPE_ARP);
	formatHeaderBeg(buffer);
	buffer << OPER_REQ;
	buffer.appendHw(nw::getHwAddr());
	buffer << nw::getEthAddr();
	buffer.appendHw((HwAddr)0xFFFFFFFFFFFF);
	buffer << addr;
	return buffer;
}

Bytes& formatReply(Bytes& buffer, HwAddr macTo, Ipv4Addr ipTo) {
	nw::fillEthernetHeader(buffer, macTo, nw::ETHERTYPE_ARP);
	formatHeaderBeg(buffer);
	buffer << OPER_REPL;
	buffer.appendHw(nw::getHwAddr());
	buffer << nw::getEthAddr();
	buffer.appendHw(macTo);
	buffer << ipTo;
	return buffer;
}

} // END NAMESPACE
