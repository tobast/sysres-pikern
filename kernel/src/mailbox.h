/****
 * Implements the "mailbox" protocol, which allows communication between the
 * ARM core and its hardware to retrieve a few hardware-related values
 * (eg. MAC address, board revision, ...)
 ****/

#pragma once

/// For protocol infos, see
/// [1] https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
/// and
/// [2] http://raspberryalphaomega.org.uk/2013/01/13/
///		how-to-read-raspberry-pi-board-revision-and-memory-size/

#include <cstdint>

namespace mailbox {

class WrongAlignment{};

// =============== ACCESS HARDWARE INFOS =====================
uint32_t getBoardModel();
// Returns the board model.
uint32_t getBoardRevision();
// Returns the board revision.

uint64_t getMac();
// Stores the 6 bytes of the ethernet MAC address in [out] (in network order).
uint32_t getRamSize();
// Returns the RAM size.

// =============== DEVICES POWER STATE ========================
uint32_t getPowerState(uint32_t deviceId);
/// Returns the power state of a device, in the format described on [1]
/// TL;DR: LSB = 0 if powered off, 1 if powered up + other infos.

uint32_t getPowerupTiming(uint32_t deviceId);
/// Returns the time (microseconds) needed after powering up the given device
/// for the power to be stable.

uint32_t setPowerState(uint32_t deviceId, uint32_t powerStatus);
/// Sets the power state of [deviceId] to [powerStatus] (see [1])

// ============== HARDWARE MEASUREMENTS =======================
uint32_t getCpuTemp();
/// Returns the CPU temperature (0.001th of degree C)

uint32_t getCriticalCpuTemp();
/// Returns the CPU critical temperature (0.001th of degree C)

// ============== LOW-LEVEL FUNCTIONS =========================

void readTag(uint32_t volatile* buffer, uint32_t timeout=0);
/// Reads the tag passed in [buffer], responding in [buffer] itself.
/// [buffer] must be 16-bytes aligned, else WrongAlignment is thrown.
/// If after more than [timeout] microseconds, no answer is received, calls
/// crash() (give 0 for no timeout).

} // END NAMESPACE

