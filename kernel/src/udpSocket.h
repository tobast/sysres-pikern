#pragma once

#include "genericSocket.h"

class UdpSocket : protected GenericSocket {
	public:
		struct PckInfo {
			PckInfo() {}
			PckInfo(Ipv4Addr fromAddr, uint16_t fromPort, bool whole=true):
				fromAddr(fromAddr), fromPort(fromPort), whole(whole) {}
			Ipv4Addr fromAddr;
			uint16_t fromPort;
			bool whole;
		};

		UdpSocket(bool blockingState, unsigned maxSize=DFT_MAX_SIZE);
		/** See GenericSocket::GenericSocket */

		unsigned write(const void* inData, unsigned len,
				Ipv4Addr fromAddr, uint16_t fromPort);
		/** Writes an entire packet to the socket, reading its [len] bytes from
		 * [inData].
		 * Returns 0 if it failed, or [len]. */

		unsigned read(void* buff, unsigned maxSize, PckInfo& infos);
		/** Reads a packet from the socket, writing at most [maxSize] bytes
		 * of it to [buff] and copying its infos to [infos].
		 * Returns the number of bytes read.
		 **/

		bool isFull() const { return GenericSocket::isFull(); }
		bool isEmpty() const { return GenericSocket::isEmpty(); }
		bool isBlocking() const { return GenericSocket::isBlocking(); }

	private:
		Queue<PckInfo> pckInfos;
};

