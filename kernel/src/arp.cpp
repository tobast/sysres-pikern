#include "malloc.h"
#include "arp.h"
#include "hashTable.h"
#include "sleep.h"

namespace arp {

const uint16_t HTYPE=1,
	  PTYPE=0x0800,
	  OPER_REQ=1, OPER_REPL=2;
const uint8_t HLEN=6,
	  PLEN=4;

enum ArpStatus {
	ARP_SENT, ARP_KNOWN
};
struct ArpData {
	ArpData(HwAddr addr) : status(ARP_KNOWN), addr(addr) {}
	ArpData() : status(ARP_SENT) {
		sentTime = elapsed_us();
	}
	ArpStatus status;
	union {
		uint64_t sentTime ;
		HwAddr addr;
	};
};

// ARP cache: keeps record of every previously-seen MAC-IPv4 match.
HashTable<Ipv4Addr, ArpData> arpCache;

HwAddr cachedHwAddr(const Ipv4Addr& addr) {
//	return 0x17298ce53b6c;
	try {
		ArpData data = arpCache.find(addr);
		switch(data.status) {
			case ARP_KNOWN:
				return data.addr;
			case ARP_SENT:
				//FIXME resend
				return 0;
			default:
				return 0;
		}
	} catch(HashTable<Ipv4Addr,ArpData>::NotFound) {
		queryArp(addr);
		return 0;
	}
}

void queryArp(const Ipv4Addr& addr) {
	arpCache.insert(addr, ArpData());

	Bytes pck;
	formatQuery(pck, addr);
	nw::sendFrame(pck, true);
	Bytes log;
	log << "Formatted ARP query:\n";
	pck.hexdump(log);
	appendLog(log);
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

	/*
	appendLog(LogDebug, "ARP", "Valid ARP: %d %d %d %d %d from %I (%M)"
		   "to %I (%M).",
			htype, ptype, hlen, plen, oper,
			ipFrom, hwFrom, ipTo, hwTo);
	*/
	arpCache.insert(ipFrom, ArpData(hwFrom));
	if(oper == OPER_REQ && ipTo == nw::getEthAddr()) {
		Bytes repl;
		formatReply(repl, hwFrom, ipFrom);
		nw::sendFrame(repl);
	}
}

Bytes& formatQuery(Bytes& buffer, Ipv4Addr addr) {
	nw::fillEthernetHeader(buffer, (HwAddr)0xFFFFFFFFFFFF,
			nw::ETHERTYPE_ARP);
	formatHeaderBeg(buffer);
	buffer << OPER_REQ;
	buffer.appendHw(nw::getHwAddr());
	buffer << nw::getEthAddr();
	buffer.appendHw((HwAddr)0x00);
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

void init() {
	arpCache.init();
}

} // END NAMESPACE
