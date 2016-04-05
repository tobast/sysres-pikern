#ifndef DEF_NW_BYTES
#define DEF_NW_BYTES

#include <cstdlib>

class Bytes {
	public:
		class OutOfRange {};
		/// Exception -- thrown by operator[]

		Bytes();

		char& operator[](const size_t pos);
		/// Accesses the [pos]th element. Throws OutOfRange if pos >= size()
		
		size_t size() const;
		/// Returns the number of chars in the object.
	
	private: //meth
		void append_bytes(char* bytes, size_t num);
		/// Appends [bytes] to [data]. [bytes] must have size [num].

	private:
		char* data;
		size_t len, reserved;
};

#endif//DEF_NW_BYTES
