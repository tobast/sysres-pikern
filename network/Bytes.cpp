#include "Bytes.h"

Bytes::Bytes() : data(), len(0), reserved(0), firstIndex(0) {}

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
	return len - firstIndex;
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

Bytes Bytes::sub(size_t beg, size_t len) const {
	if(beg+len >= size())
		throw (new OutOfRange);
	Bytes out;
	for(size_t byte=0; byte < len; byte++)
		out << at(byte+beg);
	return out;
}

template<typename T> void Bytes::extractData(T& v) {
	if(size() < sizeof(v))
		throw (new OutOfRange);
	for(size_t byte=0; byte < sizeof(v); byte++)
		v = (v<<8) | data[firstIndex+byte];
	firstIndex += sizeof(v);
}

template<typename T> void Bytes::insertData(T v) {
	for(size_t byte=sizeof(v)-1; byte >= 0; byte--)
		data.push_back((uint8_t) v >> (8*byte));
}

