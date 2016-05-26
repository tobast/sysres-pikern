#include "udpSocket.h"

UdpSocket::UdpSocket(bool blockingState, unsigned maxSize) :
	GenericSocket::GenericSocket(blockingState, maxSize) {}

unsigned UdpSocket::write(const void* inData, unsigned len,
		Ipv4Addr fromAddr, uint16_t fromPort)
{
	unsigned out = GenericSocket::write(inData, len, true, true);
	if(out != 0)
		pckInfos.push(PckInfo(fromAddr, fromPort));
	return out;
}

unsigned UdpSocket::read(void* buff, unsigned maxSize, PckInfo& infos) {
	bool atDelim = false;
	unsigned len = GenericSocket::read(buff, maxSize, &atDelim);
	infos.whole = atDelim;
	infos.fromAddr = pckInfos.front().fromAddr;
	infos.fromPort = pckInfos.front().fromPort;
	
	if(atDelim)
		pckInfos.pop();
	return len;
}

