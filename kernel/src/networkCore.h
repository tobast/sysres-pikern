#pragma once

#include "common.h"
#include "malloc.h"
#include "sleep.h"
#include "mailbox.h"
#include "writeBuffer.h"
#include "Bytes.h"
#include <uspi.h>

namespace nw {
	const uint16_t ETHERTYPE = 0x0800;

	Bytes& fillEthernetHeader(Bytes& buffer, HwAddr destMac);
	/** Fills the ethernet layer header with the given [destMac] in [buffer].
	 * Returns a reference to [buffer] to allow chaining.
	 */

	Ipv4Addr getEthAddr();
	/** Returns the IPv4 addr currently in use by the ethernet interface. */

	HwAddr getHwAddr();
	/** Returns the hardware (MAC) address of the ethernet interface. */	

	int sendFrame(const Bytes& frame);
	/** Sends a frame over Ethernet. Returns 0 on failure. */

	int pollFrame(Bytes& frame);
	/** Receives a frame over Ethernet, filling [frame].
	 * Returns 0 if no frame was available, or on failure. */

	void packetHandlerStart();
	/** Infinite loop: polls packets, replies to ARP requests,
	 * redistributes packets to processes.
	 **/
} // END NAMESPACE
