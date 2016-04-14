#ifndef DEF_NW_UDPSOCKET
#define DEF_NW_UDPSOCKET

#include <queue>
#include <vector>

#include "../Bytes.h"
#include "Ipv4Decorator.h"

namespace network {

void __SEND__(const Bytes&); // TODO IMPLEMENT (not here, though.)

class UdpSocket {
	public:
		typedef uint16_t Port;
		typedef Ipv4Decorator::Ipv4Addr Addr;

		class TooLargePacket {};
		/// Thrown if a packet is too large to be sent.

		UdpSocket(Addr saddr, Port sport, Addr daddr, Port dport);
		/// Creates a fresh new UdpSocket, emitting packets from the address
		/// [saddr]:[sport] and sending them to [daddr]:[sport].

		void send(const Bytes& data);
		/// Sends the packet [data] to the peer.
		
		void bytesRecv(const Bytes& data);
		/// Called by the kernel whenever a data fragment is received.
		
		bool pollPacket(Bytes& data);
		/// Polls the received packets for a pending packet, and writes it to
		/// [data] if one is readable.
		/// If a packet was actually read, returns [true], else [false].

	protected: //meth
		struct Packet {
			Port sport,dport;
			uint16_t pckLen, chksum;
			Bytes data;
		};

		Bytes decorate(const Bytes& pck);
		Packet extract(const Bytes& pck);

	private:
		Addr saddr,daddr;
		Port sport,dport;

		std::queue<Bytes> pendingPackets;

		Ipv4Decorator ipv4Decorator;
};

} // END NAMESPACE

#endif//DEF_NW_UDPSOCKET
