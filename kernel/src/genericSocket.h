#pragma once

/**
 * Generic socket: allows reading/writing chars, placing packet delimiters, ...
 **/

#include "common.h"
#include "queue.h"

const int DFT_MAX_SIZE=4096;

class GenericSocket {
	public:
		class Empty {}; // Raised by [readByte] on an empty queue.

		GenericSocket(bool blockingState, unsigned maxSize=DFT_MAX_SIZE);
		/**
		 * If [blockingState] is true, trying to write to a full socket will
		 * lead to an assert failure. *ALWAYS* check that a blocking socket
		 * is not full before writing...
		 * If [blockingState] is false, trying to write to a full socket will
		 * drop the packet.
		 **/

		unsigned write(const void* inData, unsigned len, bool addDelim=false,
				bool writeWhole=false);
		/**
		 * Writes the first [len] bytes of [inData] to the socket. If the
		 * socket is full, will act depending on the socket's state (see
		 * constructor). If the socket has not enough free space to write
		 * [len] bytes, all bytes that can be written will be.
		 * If [addDelim] is true, [writeDelimiter()] will be called after
		 * writing the [len] bytes.
		 * If [writeWhole] is true, writes everything if possible, or nothing.
		 * Returns the number of bytes actually written.
		 **/
		void writeByte(uint8_t v);
		/** Same as [write], but for one character only. */
		void writeDelimiter();
		/**
		 * Adds a packet delimiter at the current position, which will
		 * force reading methods to stop at this point, rather than
		 * concatenate two unrelated packets in one call.
		 **/

		unsigned read(void* buff, unsigned maxSize, bool* atDelim=NULL);
		/**
		 * Tries to read at most [maxSize] bytes to [buff]. Stops on an
		 * empty socket, or when a delimiter is encountered.
		 * If [atDelim != NULL], sets [atDelim] to true if the reading was
		 * stopped by a delimiter.
		 **/
		uint8_t readByte();
		/** Reads a single byte from the socket. Raises [Empty] if empty.
		 * Fails on assert if there is a delimiter at front.
		 **/

		bool isFull() const;
		bool isEmpty() const;
		bool isBlocking() const;

	private:
		bool blocking;
		unsigned maxSize, sinceLastDelim;

		Queue<uint8_t> data;
		Queue<unsigned> delims;
};

