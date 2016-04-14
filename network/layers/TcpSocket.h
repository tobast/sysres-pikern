#ifndef DEF_NW_TCPSOCKET
#define DEF_NW_TCPSOCKET

#include <vector>

#include "../Bytes.h"
#include "Ipv4Decorator.h"

namespace network {

class TcpSocket {
	public:
		typedef uint16_t Port;
		typedef Ipv4Decorator::Ipv4Addr Addr;
		enum State {
			CLOSED,SYN_SENT,SYN_RECV,ESTABLISHED,CLOSING
		};

		class NotEstablished {};
		/// Thrown when requesting an action only valid in [ESTABLISHED] state,
		/// and the current state is different.

		TcpSocket(Addr saddr, Port sport);
		/// Creates a fresh new TcpSocket, emitting packets from the address
		/// [saddr]:[sport].

		void connect(Addr daddr, Port dport);
		/// Initiates a connection process to [daddr]:[dport]

		void send(const Bytes& data);
		/// Sends the packet [data] to the peer. Fails throwing
		/// [NotEstablished] in case the state is not [ESTABLISHED].
		
		void bytesRecv(const Bytes& data);
		/// Called by the kernel whenever a data fragment is received.
		
		bool pollPacket(Bytes& data);
		/// Polls the received packets for a pending packet, and writes it to
		/// [data] if one is readable.
		/// If a packet was actually read, returns [true], else [false].

	protected: //meth
		static const uint16_t MAX_WINDOW_SIZE;
		struct Packet {
			Port sport,dport;
			uint32_t seq, ack;
			uint16_t chksum, urgPtr, winSize;
			bool f_ns,f_cwr,f_ece,f_urg,f_ack,f_psh,f_rst,f_syn,f_fin;
		};

		Packet extract(const Bytes& pck);

	private:
		Addr saddr,daddr;
		Port sport,dport;

		State state;

		std::vector<Bytes> pendingPackets;

		Ipv4Decorator ipv4Decorator;
};

} // END NAMESPACE

#endif//DEF_NW_TCPSOCKET
