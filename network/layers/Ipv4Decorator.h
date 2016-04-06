#ifndef DEF_NW_IPV4
#define DEF_NW_IPV4

#include <unordered_map>
#include <cstdint>
#include <functional>
#include "../Bytes.h"

class Ipv4Decorator {
	public:
		typedef uint32_t Ipv4Addr;

		class IncompletePacket {};
		/// Thrown when trying to extract a packet that is not complete yet.

		Ipv4Decorator(Ipv4Addr sourceIp);
		/// Creates a decorator using the given IP as source of the packets.

		Bytes decorate(const Bytes& data, Ipv4Addr destIp);
		/// Decorates the given [data] with an IPv4 header, ready to be sent
		/// to [destIp].
	
		Bytes extract(const Bytes& data);
		/// Extracts the data from the IPv4 decorated packet [data]. If the
		/// packet is fragmented and is not fully received yet, this function
		/// throws [IncompletePacket].
	
	private:
		typedef uint16_t Uuid;
		typedef uint8_t uchar;
		typedef uint16_t ushort;

		struct UuidHash {
			size_t operator()(const Uuid& v) const;
		};

		std::unordered_map<Uuid,Bytes,UuidHash> uncompleted;
		/// Keeps the packets that are not yet fully received.
		Uuid uuid;
		/// Current packet UUID, incremented each time.

		Ipv4Addr sourceIp;
		/// Address from which we are sending packets.
};

#endif//DEF_NW_IPV4
