#ifndef DEF_NW_IPV4
#define DEF_NW_IPV4

#include <unordered_map>
#include <cstdint>
#include <functional>
#include <set>
#include <vector>
#include "../Bytes.h"

namespace network {

const uint8_t IPV4_PROTO_TCP = 0x06;
const uint8_t IPV4_PROTO_UDP = 0x11;

class Ipv4Decorator {
	public:
		typedef uint32_t Ipv4Addr;

		class IncompletePacket {};
		/// Thrown when trying to extract a packet that is not complete yet.
		class InvalidPacket {};
		/// Thrown when a packet is invalid or corrupted.
		class TooLargePacket {};
		/// Thrown when trying to decorate a packet without fragmentation that
		/// cannot fit in a single packet.
		
		struct Packet {
			Bytes data;
			Ipv4Addr addr;
			uint8_t proto;
		};
		struct RawDatagram {
			std::vector<Bytes> data;
			Ipv4Addr addr;
			uint8_t proto;
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
	
		Packet extract(const Bytes& packet);
		/// Extracts the data from the IPv4 decorated packet [data]. If the
		/// packet is fragmented and is not fully received yet, this function
		/// throws [IncompletePacket].
		/// Throws [InvalidPacket] if the packet is corrupted, uses the wrong
		/// protocol, ...
		
		RawDatagram extractRaw(const Bytes& packet);
		/// Does the same as [extract], but does not concatenate the packets
		/// in one single [Bytes], leaving them in a [vector<Bytes>].

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
		struct DataFragment {
			size_t offset;
			Bytes data;
			bool operator<(const DataFragment& oth) const {
				return offset < oth.offset;
			}
		};
		struct DatagramFragment {
			DatagramFragment() : lastRecv(false) {}
			std::set<DataFragment> frags;
			bool lastRecv;
		};

		typedef
			std::unordered_map<FragmentIdentifier,DatagramFragment,
				FragmentIdentifierHash>
			FragmentIdentifierHashtbl;

		FragmentIdentifierHashtbl uncompleted;
		/// Keeps the packets that are not yet fully received.
		Uuid uuid;
		/// Current packet UUID, incremented each time.

		Ipv4Addr sourceIp;
		/// Address from which we are sending packets.
};

} // END NAMESPACE

#endif//DEF_NW_IPV4
