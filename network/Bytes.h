#ifndef DEF_NW_BYTES
#define DEF_NW_BYTES

#include <cstdint>

#include "common.h"
#include "expArray.h"

class Bytes {
	public:
		class OutOfRange {};
		/// Exception -- thrown by operator[]

		Bytes();
		Bytes(const void* buff, size_t len);
		/// Initializes the object with [buff], reading [len] bytes.

		uint8_t& operator[](const size_t pos);
		/// Accesses the [pos]th element. Throws OutOfRange if pos >= size()
		
		uint8_t at(const size_t pos) const;
		/// Same as operator[], but this is a const method.
		uint16_t ushortAt(const size_t pos) const;
		/// Accesses the 16 bits unsigned integet at position [pos].
		/// Throws OutOfRange if [pos+15] >= size().
		
		size_t size() const;
		/// Returns the number of chars in the object.
		
		Bytes& operator<<(char v);
		Bytes& operator<<(uint8_t v);
		Bytes& operator<<(uint16_t v);
		Bytes& operator<<(uint32_t v);
		Bytes& operator<<(const Bytes& v);
		Bytes& operator<<(const char* str);
		/// Appends the given data to the vector. Returns *this to allow
		/// chaining.

		Bytes& appendHw(HwAddr v);
		/// Appends a hardware address to the vector.
		
		Bytes& operator>>(uint8_t& v);
		Bytes& operator>>(uint16_t& v);
		Bytes& operator>>(uint32_t& v);
		/// Extracts the given data type from the vector. Returns *this to
		/// allow chaining.
		
		Bytes& extractHw(HwAddr& v);
		/// Extracts a hardware address from the vector.
		
		void operator=(const Bytes& oth);
		/// Copies the given Bytes object into its own data.

		Bytes sub(size_t beg, size_t len) const;
		/// Extracts a sub-bytes, beginning at character [beg], containing
		/// [len] chars. If [beg+len] >= [size()], throws OutOfRange.
		
		void writeToBuffer(void* buff) const;
		/// Writes the contents of this array in the buffer [buff].
		/// The user must provide a buffer of at least [size()] bytes.
	
		ExpArray<uint8_t> data;
	private: //meth
		template<typename T> void insertData(T v);
		template<typename T> void extractData(T& v);

	private:
		size_t firstIndex;
};

#endif//DEF_NW_BYTES
