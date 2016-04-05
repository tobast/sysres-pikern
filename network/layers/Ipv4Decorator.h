#ifndef DEF_NW_IPV4
#define DEF_NW_IPV4

#include <unordered_map>
#include <cstdint>
#include "../Bytes.h"

class Ipv4Decorator {
	public:
		typedef int32_t Ipv4Addr;

		class IncompletePacket {};
		/// Thrown when trying to extract a packet that is not complete yet.

		Ipv4Decorator(Ipv4Addr sourceIp);
		Bytes decorate(const Bytes& data, Ipv4Addr destIp);
		Bytes extract(const Bytes& data);
	
	private:
		typedef char16_t Uuid;
		typedef unsigned char uchar;

		std::unordered_map<Uuid,Bytes> uncompleted;
		Uuid uuid;

		Ipv4Addr sourceIp;
};

#endif//DEF_NW_IPV4
