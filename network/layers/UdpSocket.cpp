#include "UdpSocket.h"

namespace network {

UdpSocket::UdpSocket(Addr saddr, Port sport, Addr daddr, Port dport) :
	saddr(saddr),daddr(daddr),sport(sport),dport(dport),
	ipv4Decorator(saddr)
{}

void UdpSocket::send(const Bytes& data) {
	if(data.size() > Ipv4Decorator::maxPacketLoad() - 8) {// Must be fragmented
		//TODO implement
		throw new Ipv4Decorator::TooLargePacket;
	}
	
	Bytes packet = decorate(data);
	__SEND__(packet);
}

void UdpSocket::bytesRecv(const Bytes& data) {
	Packet pck = extract(data);
	if(pck.sport != dport || pck.dport != sport)
		return; // Drop packet.

	if(pck.pckLen != pck.data.size() - 8)
		return; // Drop packet.

	pendingPackets.push(pck.data);
}

bool UdpSocket::pollPacket(Bytes& data) {
	if(pendingPackets.empty())
		return false;
	
	Bytes front = pendingPackets.front();
	pendingPackets.pop();

	data = front;
	return true;
}

Bytes UdpSocket::decorate(const Bytes& pck) {
	Bytes out;
	out << sport << dport << (uint16_t)(pck.size()+8) << (uint16_t)0 << pck;
	return out;
}
UdpSocket::Packet UdpSocket::extract(const Bytes& pck) {
	Bytes pckCopy = pck;
	Packet out;
	pckCopy >> out.sport >> out.dport >> out.pckLen >> out.chksum;
	out.data = pckCopy;
	return out;
}

} // END NAMESPACE
