#ifndef DEF_NW_IPV4
#define DEF_NW_IPV4

#include <unordered_map>
#include <cstdint>
#include <functional>
#include <vector>
#include "../Bytes.h"

class Ipv4Decorator {
	public:
		typedef uint32_t Ipv4Addr;

		class IncompletePacket {};
		/// Thrown when trying to extract a packet that is not complete yet.
		class TooLargePacket {};
		/// Thrown when trying to decorate a packet without fragmentation that
		/// cannot fit in a single packet.
		
		struct Packet {
			Bytes data;
			Ipv4Addr addr;
		};

		Ipv4Decorator(Ipv4Addr sourceIp);
		/// Creates a decorator using the given IP as source of the packets.

		Bytes decorateSingle(const Packet& packet,
				bool notLastFragment = false);
		/// Decorates the given [data] with an IPv4 header, ready to be sent
		/// to [packet.addr]. This function returns a single decorated packet,
		/// thus [packet.data] must have at most [maxPacketLoad()] bytes.
		/// If [data] is too large, throws [TooLargePacket].
		/// If this function is used to decorate a fragment of a datagram, and
		/// this data fragment is *not* the last of the datagram,
		/// [notLastFragment] must be [true].
		
		std::vector<Bytes> decorate(const Packet& packet);
		/// Decorates the given [acket.data] with an IPv4 header,
		/// ready to be sent to [packet.addr]. Fragments [data] into
		/// several packets if needed.
	
		Bytes extract(const Bytes& data);
		/// Extracts the data from the IPv4 decorated packet [data]. If the
		/// packet is fragmented and is not fully received yet, this function
		/// throws [IncompletePacket].

		static size_t maxPacketLoad();
		/// Returns the maximum size of the data segment of a packet before
		/// it must be fragmented. Useful for protocol decorators.
	
	private: //meth
		uint16_t checksum(const Bytes& header) const;
	
	private:
		typedef uint16_t Uuid;
		typedef uint8_t uchar;
//		typedef uint16_t ushort; // Conflicts with std types

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
