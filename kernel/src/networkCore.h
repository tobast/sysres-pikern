#pragma once

#include "common.h"
#include "malloc.h"
#include "sleep.h"
#include "mailbox.h"
#include "Bytes.h"
#include "arp.h"
#include "udp.h"
#include "queue.h"
#include <uspi.h>

namespace nw {
	const uint16_t ETHERTYPE_IPV4 = 0x0800;
	const uint16_t ETHERTYPE_ARP = 0x0806;

	Bytes& fillEthernetHeader(Bytes& buffer, HwAddr destMac,
			uint16_t etherType = ETHERTYPE_IPV4);
	/** Fills the ethernet layer header with the given [destMac] in [buffer].
	 * Returns a reference to [buffer] to allow chaining.
	 */

	Ipv4Addr getEthAddr();
	/** Returns the IPv4 addr currently in use by the ethernet interface. */

	HwAddr getHwAddr();
	/** Returns the hardware (MAC) address of the ethernet interface. */	

	void logAppend(const char* l);
	void logAppend(const Bytes& b);

	void sendPacket(Bytes packet, Ipv4Addr to);
	/** Queues a layer 3-formatted packet (ie. WITHOUT Ethernet decoration)
	 * for sending. Async.
	 */

	int sendFrame(const Bytes& frame);
	/** Sends a frame over Ethernet. Returns 0 on failure. */

	int pollFrame(Bytes& frame);
	/** Receives a frame over Ethernet, filling [frame].
	 * Returns 0 if no frame was available, or on failure. */

	void init();

	void packetHandlerStart();
	/** Infinite loop: polls packets, replies to ARP requests,
	 * redistributes packets to processes.
	 **/
} // END NAMESPACE
