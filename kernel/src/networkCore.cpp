#include "networkCore.h"

namespace nw {

const unsigned PACKET_TRIES_TIMEOUT = 1000; // 0.1s

void processPacket(Bytes frame) {
	HwAddr toMac, fromMac;
	uint16_t etherType;
	frame.extractHw(fromMac);
	frame.extractHw(toMac);
	frame >> etherType;
	
	if(toMac != getHwAddr() && toMac != 0 && toMac != 0xFFFFFFFFFFFF)
		return; // None of my business

	switch(etherType) {
		case ETHERTYPE_ARP:
			arp::readArp(frame);
			break;
		case ETHERTYPE_IPV4:
			//TODO
			break;
	}
}


Bytes& fillEthernetHeader(Bytes& buffer, HwAddr destMac, uint16_t etherType) {
	buffer.appendHw(destMac).appendHw(getHwAddr());
	buffer << etherType;
	return buffer;
}

Ipv4Addr getEthAddr() {
	return 0x0a00000f; //TODO
}

HwAddr getHwAddr() {
	return mailbox::getMac(); // Caches the MAC address.
}

char* logs = NULL;
bool ignoreLogs = false;
unsigned logPos=0, logSentEnd=0;
void logAppend(const char* l) {
	if(ignoreLogs)
		return;

	unsigned len = str_len(l);
	for(unsigned pos=0; pos < len; pos++) {
		logs[logPos] = l[pos];
		logPos = (logPos+1)%0x1000;
	}
}

struct QueuedPacket {
	QueuedPacket(const Bytes& pck, const Ipv4Addr& to) :
		pck(pck),to(to),triedSend(0) {}
	Bytes pck;
	Ipv4Addr to;
	unsigned triedSend;

	QueuedPacket& operator=(const QueuedPacket& oth) {
		pck = oth.pck;
		to = oth.to;
		triedSend = oth.triedSend;
		return *this;
	}
};
Queue<QueuedPacket> sendingQueue;

void sendPacket(Bytes packet, Ipv4Addr to) {
	// Get MAC address
	HwAddr mac = arp::cachedHwAddr(to);
	if(mac == 0) /* Not cached */ {
		if(arp::queryArp(to) == 0) // ARP send failed
			assert(false, 0x55);
	}
	sendingQueue.push(QueuedPacket(packet,to));
}

int sendFrame(const Bytes& frame) {
	void* buff = malloc(frame.size()+2);
	frame.writeToBuffer(buff);
	int out = USPiSendFrame(buff, frame.size());
	free(buff);
	return out;
}

bool sendQueuedPacket(QueuedPacket& pck) {
	HwAddr mac = arp::cachedHwAddr(pck.to);
	if(mac == 0) {
		pck.triedSend++;
		return false;
	}
	Bytes frame;
	fillEthernetHeader(frame, mac);
	frame << pck.pck;
	return (sendFrame(frame) != 0);
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
	/*
//	logs = (char*)malloc(0x1000);
void packetHandlerStart() {
	Bytes frame;

	while(true) {
		if(pollFrame(frame) > 0)
			processPacket(frame);
		
		if(sendingQueue.size() > 0) {
			QueuedPacket& front = sendingQueue.front();
			if(sendQueuedPacket(front))
				sendingQueue.pop();
			else if(front.triedSend > PACKET_TRIES_TIMEOUT)
				sendingQueue.pop();
		}
		*/
		sleep_us(1000);
		/*
		if(logSentEnd != logPos) {
			ignoreLogs = true;
			Bytes pck;
			fillEthernetHeader(pck, 0x6c3be58c2917);
			Bytes payload(logs+logPos, _min(logPos-logSentEnd, 200));
			udp::formatPacket(pck, payload, 1, 0x0a000001, 3141);
			logSentEnd += _min(logPos-logSentEnd, 200);
			void* buff = malloc(pck.size());
			pck.writeToBuffer(buff);
//			USPiSendFrame(buff, pck.size());
			ignoreLogs = false;
		}
	}
	*/
		sleep_us(100);
	}
}

} // END NAMESPACE

