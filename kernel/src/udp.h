namespace udp {
	unsigned formatPacket(void* packet, const void* data, unsigned length,
			uint32_t fromAddr, uint16_t fromPort,
			uint32_t toAddr, uint16_t toPort);
	/** Writes an UTF-8 formatted packet in [packet], containing the
	 * first [length] bytes of [data] as its data. Returns the size of
	 * [packet] after being written.
	 **/

}
