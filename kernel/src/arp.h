#pragma once

//#include <map>

#include "common.h"
#include "networkCore.h"
#include "Bytes.h"

namespace arp {
	Bytes& formatQuery(Bytes& buffer, Ipv4Addr addr);
	/** Fills [buffer] with an ARP request querying [addr] for its MAC
	 * address. Returns [buffer] itself.
	 */

	Bytes& formatReply(Bytes& buffer, HwAddr macTo, Ipv4Addr ipTo);
	/** Fills [buffer] with an ARP reply for the IP [ipTo], MAC [macTo].
	 * Returns [buffer] itself.
	 */
}
