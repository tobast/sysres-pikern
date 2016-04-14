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

}

void UdpSocket::bytesRecv(const Bytes& data) {
	//TODO
}

bool UdpSocket::pollPacket(Bytes& data) {
	//TODO
	return false;
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
