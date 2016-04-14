#include "Bytes.h"

Bytes::Bytes() : data(), firstIndex(0) {}

Bytes::Bytes(void* buff, size_t len) : firstIndex(0) {
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

