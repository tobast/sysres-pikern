#include "networkCore.h"

void processPacket(const Bytes& frame) {
}

namespace nw {

Bytes& fillEthernetHeader(Bytes& buffer, HwAddr destMac) {
	buffer.appendHw(destMac).appendHw(getHwAddr());
	buffer << ETHERTYPE;
	return buffer;
}

Ipv4Addr getEthAddr() {
	return 0x0a00000f; //TODO
}

HwAddr getHwAddr() {
	return mailbox::getMac(); // Caches the MAC address.
}

int sendFrame(const Bytes& frame) {
	void* buff = malloc(frame.size()+2);
	frame.writeToBuffer(buff);
	int out = USPiSendFrame(buff, frame.size());
	free(buff);
	return out;
}

int pollFrame(Bytes& frame) {
	static void* buff = NULL;
	if(buff == NULL)
		malloc(USPI_FRAME_BUFFER_SIZE+100);
	unsigned resultLen = 0;
	int out = USPiReceiveFrame(buff, &resultLen);
	if(out != 0)
		frame = Bytes(buff, resultLen);
	return out;
}

void packetHandlerStart() {
	Bytes frame;

	while(true) {
		if(pollFrame(frame) > 0)
			processPacket(frame);
		sleep_us(100);
	}
}

} // END NAMESPACE

