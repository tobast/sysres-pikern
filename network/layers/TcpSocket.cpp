#include "TcpSocket.h"

namespace network {

const uint16_t TcpSocket::MAX_WINDOW_SIZE = (1<<16)-1;

TcpSocket::TcpSocket(TcpSocket::Addr saddr, TcpSocket::Port sport) :
	saddr(saddr),sport(sport),state(CLOSED), ipv4Decorator(saddr)
{}

void TcpSocket::connect(TcpSocket::Addr daddr, TcpSocket::Port dport) {
	this->daddr = daddr;
	this->dport = dport;
	// TODO
}

void TcpSocket::send(const Bytes& data) {
	if(state != ESTABLISHED)
		throw new NotEstablished;

	Bytes out;
	//TODO
}

void TcpSocket::bytesRecv(const Bytes& data) {
	try {
		Ipv4Decorator::RawDatagram datagram = ipv4Decorator.extractRaw(data);

		if(datagram.addr != saddr || datagram.proto != IPV4_PROTO_TCP)
			return; // Dropped.
		
		for(auto pckIt : datagram.data) {
			Packet pck = extract(pckIt);
			if(pck.dport != sport)
				return; // Dropped.
		}
	} catch(const Ipv4Decorator::IncompletePacket& e) {}
}

bool TcpSocket::pollPacket(Bytes& data) {
	// TODO
	return false;
}

TcpSocket::Packet TcpSocket::extract(const Bytes& pck) {
	Packet out;
	//TODO
	return out;
}

} // END NAMESPACE

