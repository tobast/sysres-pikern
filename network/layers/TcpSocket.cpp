#include "TcpSocket.h"

namespace network {

TcpSocket::TcpSocket(TcpSocket::Addr saddr, TcpSocket::Port sport) :
	saddr(saddr),sport(sport),state(CLOSED)
{}

void TcpSocket::connect(TcpSocket::Addr daddr, TcpSocket::Port dport) {
	
}

void TcpSocket::send(const Bytes& data) {
	// TODO
}

void TcpSocket::bytesRecv(const Bytes& data) {
	// TODO
}

bool TcpSocket::pollPacket(Bytes& data) {
	// TODO
}

} // END NAMESPACE

