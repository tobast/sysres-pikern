#pragma once

//#include <map>

#include "common.h"
#include "networkCore.h"
#include "Bytes.h"

namespace arp {
	HwAddr cachedHwAddr(const Ipv4Addr& addr);
	/** Returns the cached hardware address of [addr] if previously cached,
	 * or 0.
	 */

	int queryArp(const Ipv4Addr& addr);
	/** Sends an ARP query for [addr]. */

	void readArp(Bytes arp);
	/** Reads [arp] and takes the appropriate action, which might include
	 * sending a packet back.
	 **/

	Bytes& formatQuery(Bytes& buffer, Ipv4Addr addr);
	/** Fills [buffer] with an ARP request querying [addr] for its MAC
	 * address. Returns [buffer] itself.
	 */

	Bytes& formatReply(Bytes& buffer, HwAddr macTo, Ipv4Addr ipTo);
	/** Fills [buffer] with an ARP reply for the IP [ipTo], MAC [macTo].
	 * Returns [buffer] itself.
	 */
}
