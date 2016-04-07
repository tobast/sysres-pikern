#ifndef DEF_NW_BYTES
#define DEF_NW_BYTES

#include <cstdlib>
#include <cstdint>

class Bytes {
	public:
		class OutOfRange {};
		/// Exception -- thrown by operator[]

		Bytes();

		char& operator[](const size_t pos);
		/// Accesses the [pos]th element. Throws OutOfRange if pos >= size()
		
		char at(const size_t pos) const;
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
		/// Appends the given data to the vector. Returns *this to allow
		/// chaining.
		
		Bytes& operator>>(char& v);
		Bytes& operator>>(uint8_t& v);
		Bytes& operator>>(uint16_t& v);
		Bytes& operator>>(uint32_t& v);

		Bytes sub(size_t beg, size_t len) const;
		/// Extracts a sub-bytes, beginning at character [beg], containing
		/// [len] chars. If [beg+len] >= [size()], throws OutOfRange.
	
	private: //meth
		void append_bytes(char* bytes, size_t num);
		/// Appends [bytes] to [data]. [bytes] must have size [num].

	private:
		char* data;
		size_t len, reserved;
};

#endif//DEF_NW_BYTES
