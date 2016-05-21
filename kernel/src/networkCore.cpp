#include "networkCore.h"
#include "atomic.h"
#include "logger.h"

#include "gpio.h" // FIXME

namespace nw {

const unsigned PACKET_TRIES_TIMEOUT = 1000; // 0.1s

void processPacket(Bytes frame) {
	HwAddr toMac, fromMac;
	uint16_t etherType;
	frame.extractHw(toMac);
	frame.extractHw(fromMac);
	frame >> etherType;

	if(toMac != getHwAddr() && toMac != 0 && toMac != 0xFFFFFFFFFFFF)
		return; // None of my business

	switch(etherType) {
		case ETHERTYPE_ARP:
			arp::readArp(frame);
			break;
		case ETHERTYPE_IPV4: {
			//TODO
			if(toMac != getHwAddr())
				break;
			try {
				udp::PckInfos infos = udp::extractHeader(frame);
				if(infos.dataSize != 3 || infos.toPort != 2)
					break;
				char c1,c2,c3;
				frame >> c1 >> c2 >> c3;
				if(c1 == 'R' && c2 == 'P' && c3 == 'i')
					logger::addListener(infos.fromAddr);
			} catch(udp::BadChecksum&) {
				appendLog(LogWarning, "udp", "Bad checksum from %M", fromMac);
			} catch(udp::WrongProtocol&) {
				appendLog(LogWarning, "udp", "Bad protocol");
			} catch(...) {
				appendLog(LogWarning, "udp", "Unexpected unknown exception.");
			}
			break;
		}
	}
}


Bytes& fillEthernetHeader(Bytes& buffer, HwAddr destMac, uint16_t etherType) {
	buffer.appendHw(destMac).appendHw(getHwAddr());
	buffer << etherType;
	return buffer;
}

Ipv4Addr getEthAddr() {
	return 0x81c79dae; // FIXME
}

HwAddr getHwAddr() {
	return mailbox::getMac(); // Caches the MAC address.
}

bool ignoreLogs=false;

struct QueuedPacket {
	QueuedPacket(const Bytes& pck, const Ipv4Addr& to) :
		pck(pck),to(to),triedSend(0),frameReady(false) {}
	QueuedPacket(const Bytes& pck, const Ipv4Addr& to, bool frameReady) :
		pck(pck),to(to),triedSend(0),frameReady(frameReady) {}
	Bytes pck;
	Ipv4Addr to;
	unsigned triedSend;
	bool frameReady;

	QueuedPacket& operator=(const QueuedPacket& oth) {
		pck = oth.pck;
		to = oth.to;
		triedSend = oth.triedSend;
		frameReady = oth.frameReady;
		return *this;
	}
};
Queue<QueuedPacket*> *sendingQueue = NULL;
mutex_t *queue_mutex = NULL;

int doSendFrame(void* buffer, unsigned len) {
	ignoreLogs = true;
	int out = USPiSendFrame(buffer, len);
	ignoreLogs = false;
	return out;
}

void sendPacket(Bytes packet, Ipv4Addr to) {
	assert(sendingQueue != NULL, 0xca);

	// Get MAC address
	arp::cachedHwAddr(to); // send ARP request
	QueuedPacket* nPacket = (QueuedPacket*)malloc(sizeof(QueuedPacket));
	*nPacket = QueuedPacket(packet, to);
	mutex_lock(queue_mutex);
	sendingQueue->push(nPacket);
	mutex_unlock(queue_mutex);
}

void sendFrame(const Bytes& frame, bool isPrio) {
	assert(sendingQueue != NULL, 0xca);
	QueuedPacket* nPacket = (QueuedPacket*)malloc(sizeof(QueuedPacket));
	*nPacket = QueuedPacket(frame, 0x00, true);
	mutex_lock(queue_mutex);
	if(isPrio)
		sendingQueue->push_front(nPacket);
	else
		sendingQueue->push(nPacket);
	mutex_unlock(queue_mutex);
}

int sendFrameNow(const Bytes& frame) {
	void* buff = malloc(frame.size()+2);
	frame.writeToBuffer(buff);
	int out = doSendFrame(buff, frame.size());
	free(buff);
	return out;
}

bool sendQueuedPacket(QueuedPacket* pck) {
	if(pck->frameReady) { // The frame is already formatted.
		return (sendFrameNow(pck->pck) != 0);
	}


	HwAddr mac = arp::cachedHwAddr(pck->to);
	if(mac == 0) {
		pck->triedSend++;
		return false;
	}
	Bytes frame;
	fillEthernetHeader(frame, mac);
	frame << pck->pck;
	return (sendFrameNow(frame) != 0);
}

int pollFrame(Bytes& frame) {
	static void* buff = NULL;
	if(buff == NULL)
		buff = malloc(USPI_FRAME_BUFFER_SIZE+100);
	unsigned resultLen = 0;
	int out = USPiReceiveFrame(buff, &resultLen);
	if(out != 0)
		frame = Bytes(buff, resultLen);
	return out;
}

void init() {
	sendingQueue = (Queue<QueuedPacket*>*)malloc(sizeof(Queue<QueuedPacket*>));
	*sendingQueue = Queue<QueuedPacket*>();

	arp::init();

	queue_mutex = (mutex_t*)(malloc(sizeof(mutex_t)));
	mutex_init(queue_mutex);
}

void logUsedIp() {
	appendLog(LogInfo, "network", "Using IPv4 with address %I", getEthAddr());
}

void packetHandlerStart() {
	assert(sendingQueue != NULL, 0xca);
	Bytes frame;

	logUsedIp();
	appendLog(LogInfo, "network", "Using MAC address %M", getHwAddr());

	while(true) {
		while(pollFrame(frame) != 0)
			processPacket(frame);

		mutex_lock(queue_mutex);
		while(sendingQueue->size() > 0) {
			QueuedPacket* front = sendingQueue->front();
			if(sendQueuedPacket(front))
				free(sendingQueue->pop());
			else if(front->triedSend > PACKET_TRIES_TIMEOUT)
				free(sendingQueue->pop());
			else
				break;
		}
		mutex_unlock(queue_mutex);
		sleep(100);
	}
}

} // END NAMESPACE

