#pragma once

#include "common.h"
#include "malloc.h"
#include "sleep.h"
#include "mailbox.h"
#include "Bytes.h"
#include "arp.h"
#include "udp.h"
#include "icmp.h"
#include "ipv4.h"
#include "queue.h"
#include "genericSocket.h"
#include <uspi.h>

namespace nw {
	const uint16_t ETHERTYPE_IPV4 = 0x0800;
	const uint16_t ETHERTYPE_ARP = 0x0806;

	class InconsistentHeaders {};

	uint16_t networkChksum(const Bytes& b, unsigned headBeg, unsigned headEnd,
			uint16_t firstShort=0);
	/** Computes the Network Checksum of the given [b], starting at its
	 * [headBeg]th byte, ending to its [headEnd-1]th byte, and adding
	 * [firstShort] to the sum, if provided.
	 **/

	GenericSocket* bindUdpPort(uint16_t port);
	/** Binds the given UDP port to a socket, returned by the function.
	 * The returned socket is NULL if the port is already bound.
	 **/

	// TODO closeUdpPort

	Bytes& fillEthernetHeader(Bytes& buffer, HwAddr destMac,
			uint16_t etherType = ETHERTYPE_IPV4);
	/** Fills the ethernet layer header with the given [destMac] in [buffer].
	 * Returns a reference to [buffer] to allow chaining.
	 */

	Ipv4Addr getEthAddr();
	/** Returns the IPv4 addr currently in use by the ethernet interface. */

	HwAddr getHwAddr();
	/** Returns the hardware (MAC) address of the ethernet interface. */	

	void sendPacket(Bytes packet, Ipv4Addr to);
	/** Queues a layer 3-formatted packet (ie. WITHOUT Ethernet decoration)
	 * for sending. Async.
	 */

	void sendFrame(const Bytes& frame, bool isPrio=false);
	/** Sends a frame over Ethernet. If isPrio=true, the frame is pushed
	 * to the *front* of the sending queue. Async.
	 **/

	int pollFrame(Bytes& frame);
	/** Receives a frame over Ethernet, filling [frame].
	 * Returns 0 if no frame was available, or on failure. */

	void init();

	void packetHandlerStart();
	/** Infinite loop: polls packets, replies to ARP requests,
	 * redistributes packets to processes.
	 **/
} // END NAMESPACE
