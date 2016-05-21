#include "Bytes.h"

Bytes::Bytes() : data(), firstIndex(0) {}

Bytes::Bytes(const void* buff, size_t len) : data(), firstIndex(0) {
	data.reserve(len);
	for(size_t pos=0; pos < len; pos++)
		data.push_back(((uint8_t*)buff)[pos]);
}

uint8_t& Bytes::operator[](const size_t pos) {
	if(pos < size())
		return data[pos+firstIndex];
	throw (new OutOfRange);
}

uint8_t Bytes::at(const size_t pos) const {
	if(pos < size())
		return data[pos+firstIndex];
	throw (new OutOfRange);
}
uint16_t Bytes::ushortAt(const size_t pos) const {
	if(pos+1 < size())
		return (((uint16_t)data[pos+firstIndex+1])<<8)+data[pos+firstIndex];
	throw (new OutOfRange);
}

size_t Bytes::size() const {
	return data.size() - firstIndex;
}

Bytes& Bytes::operator<<(char v) {
	insertData<char>(v);
	return *this;
}
Bytes& Bytes::operator<<(uint8_t v) {
	insertData<uint8_t>(v);
	return *this;
}
Bytes& Bytes::operator<<(uint16_t v) {
	insertData<uint16_t>(v);
	return *this;
}
Bytes& Bytes::operator<<(uint32_t v) {
	insertData<uint32_t>(v);
	return *this;
}
Bytes& Bytes::operator<<(const Bytes& v) {
	data.reserve(data.size() + v.size());
	for(size_t byte=0; byte < v.size(); byte++)
		data.push_back(v.at(byte));
	return *this;
}
Bytes& Bytes::operator<<(const char* str) {
	unsigned len = str_len(str);
	data.reserve(data.size() + len);
	for(unsigned byte=0; byte < len; byte++)
		data.push_back(str[byte]);
	return *this;
}

Bytes& Bytes::appendHw(HwAddr v) {
	for(int byte=0; byte < 6; byte++)
		(*this) << (uint8_t)(v >> 8*byte);
	return *this;
}

Bytes& Bytes::operator>>(uint8_t& v) {
	extractData<uint8_t>(v);
	return *this;
}
Bytes& Bytes::operator>>(uint16_t& v) {
	extractData<uint16_t>(v);
	return *this;
}
Bytes& Bytes::operator>>(uint32_t& v) {
	extractData<uint32_t>(v);
	return *this;
}

Bytes& Bytes::extractHw(HwAddr& v) {
	v=0;
	for(int byte=0; byte < 6; byte++) {
		uint8_t b;
		(*this) >> b;
		v |= ((uint64_t)b) << 8*byte;
	}
	return *this;
}

void Bytes::operator=(const Bytes& oth) {
	firstIndex=0;
	data.clear();
	data.reserve(oth.size());
	for(size_t pos=0; pos < oth.size(); pos++)
		data.push_back(oth.at(pos));
}

Bytes Bytes::sub(size_t beg, size_t len) const {
	if(beg+len >= size())
		throw (new OutOfRange);
	Bytes out;
	for(size_t byte=0; byte < len; byte++)
		out << at(byte+beg);
	return out;
}

void Bytes::writeToBuffer(void* buff) const {
	for(size_t pos=0; pos < size(); pos++)
		((uint8_t*)buff)[pos] = at(pos);
}

char charOfDigit(uint8_t dig) {
	if(dig < 10)
		return dig+'0';
	return dig+'A'-10;
}
void hexdumpByte(Bytes& dest, uint8_t b) {
	dest << charOfDigit(b/16) << charOfDigit(b & 0xf);
}

Bytes& Bytes::appendHex(const uint32_t& v) {
	for(int b=3; b >= 0; b--)
		hexdumpByte(*this, (v >> b*8) & 0xff);
	return *this;
}

void Bytes::hexdump(Bytes& dest) const {
	for(unsigned pos=0; pos < size(); pos++) {
		hexdumpByte(dest, at(pos));
		if(pos % 32 == 31)
			dest << '\n';
		else if(pos % 4 == 3)
			dest << ' ';
	}
	if(size() % 32 != 0)
		dest << '\n';
}

template<typename T> void Bytes::extractData(T& v) {
	if(size() < sizeof(v))
		throw (new OutOfRange);
	for(size_t byte=0; byte < sizeof(v); byte++)
		v = (v<<8) | data[firstIndex+byte];
	firstIndex += sizeof(v);
}

template<typename T> void Bytes::insertData(T v) {
	for(int byte=sizeof(v)-1; byte >= 0; byte--) {
		data.push_back((uint8_t) (v >> (8*byte)));
	}
}

