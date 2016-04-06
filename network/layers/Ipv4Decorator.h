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

		struct FragmentIdentifier {
			FragmentIdentifier(Ipv4Addr from, Ipv4Addr to,
					Uuid uuid, uchar protocol) :
					from(from),to(to),uuid(uuid),protocol(protocol) {}
			Ipv4Addr from, to;
			Uuid uuid;
			uchar protocol;

			bool operator==(const FragmentIdentifier& e) const {
				return (from==e.from && to==e.to && uuid==e.uuid &&
						protocol == e.protocol);
			}
		};
		struct FragmentIdentifierHash {
			size_t operator()(const FragmentIdentifier& v) const;
		};

		typedef
			std::unordered_map<FragmentIdentifier,Bytes,FragmentIdentifierHash>
			FragmentIdentifierHashtbl;

		FragmentIdentifierHashtbl uncompleted;
		/// Keeps the packets that are not yet fully received.
		Uuid uuid;
		/// Current packet UUID, incremented each time.

		Ipv4Addr sourceIp;
		/// Address from which we are sending packets.
};

#endif//DEF_NW_IPV4
