/****
 * Implements the "mailbox" protocol, which allows communication between the
 * ARM core and its hardware to retrieve a few hardware-related values
 * (eg. MAC address, board revision, ...)
 ****/

#pragma once

/// For protocol infos, see
/// https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
/// and
/// http://raspberryalphaomega.org.uk/2013/01/13/
///		how-to-read-raspberry-pi-board-revision-and-memory-size/

#include <cstdint>

namespace mailbox {

class WrongAlignment{};

uint32_t getBoardModel();
// Returns the board model.
uint32_t getBoardRevision();
// Returns the board revision.
void getMac(uint8_t* out);
// Stores the 6 bytes of the ethernet MAC address in [out] (in network order).
uint32_t getRamSize();
// Returns the RAM size.

void readTag(uint32_t volatile* buffer, uint32_t timeout=0);
/// Reads the tag passed in [buffer], responding in [buffer] itself.
/// [buffer] must be 16-bytes aligned, else WrongAlignment is thrown.
/// If after more than [timeout] microseconds, no answer is received, calls
/// crash() (give 0 for no timeout).

} // END NAMESPACE

