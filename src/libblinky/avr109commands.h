#ifndef AVR_109_COMMANDS_H
#define AVR_109_COMMANDS_H

#include "libblinkyglobal.h"

#include "serialcommand.h"

#include <QList>

/// Subset of AVR109 commands supported by the Caterina bootloader
/// See Caterina.c in the Arduino sources, and the 'AVR109: Self Programming'
/// guide by Atmel for further information.

namespace Avr109Commands {

/// Check that we are talking to the correct device
LIBBLINKY_EXPORT SerialCommand checkDeviceSignature();

/// Instruct the programmer to reset.
LIBBLINKY_EXPORT SerialCommand reset();

/// Erase the entire flash memory
LIBBLINKY_EXPORT SerialCommand chipErase();

/// Set the current read/write address for flash/EEPROM reading/writing
/// @param address Offset in the device flash space to use for the next read
/// or write operation
LIBBLINKY_EXPORT SerialCommand setAddress(unsigned int address);

/// Write a page of flash data
/// Note that the address must be set by running setAddress() before this
/// command.
/// @param data Data to write. Will only succeed if the data length is equal
/// or less than the page length on the microcontroller (128 bytes for atmega32u4)
/// Data must be word-aligned, and will be padded with 0xFF if necessicary.
LIBBLINKY_EXPORT SerialCommand writeFlashPage(const QByteArray &data);

/// Verify a page of flash data
/// Note that the address must be set by running setAddress() before this
/// command.
/// @param data Data to verify. Data must be word-aligned, and will be padded with
///  0xFF if necessicary.
LIBBLINKY_EXPORT SerialCommand verifyFlashPage(const QByteArray &data);

/// Write EEPROM data
/// Note that the address must be set by running setAddress() before this
/// command.
/// @param data Data to write. Size is not constrained.
LIBBLINKY_EXPORT SerialCommand writeEepromBlock(const QByteArray &data);


/// Write the contents of the flash
/// Note that if length is not a multiple of the page size, some pre-existing
/// data at the end may be erased.
/// @param data QByteArray containing the data to write to the flash
/// @param startAddress Page-aligned address to begin writing to, in bytes
LIBBLINKY_EXPORT QList<SerialCommand> writeFlash(const QByteArray &data, unsigned int startAddress);

/// Verify the contents of the flash
/// @param data QByteArray containing the expected flash data
/// @param startAddress Page-aligned address to verify from, in bytes
LIBBLINKY_EXPORT QList<SerialCommand> verifyFlash(const QByteArray &data, unsigned int startAddress);


LIBBLINKY_EXPORT QList<SerialCommand> writeEeprom(const QByteArray &data, unsigned int startAddress);


}

#endif // AVR_109_COMMANDS_H
