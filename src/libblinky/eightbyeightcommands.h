#ifndef EIGHTBYEIGHTCOMMANDS_H
#define EIGHTBYEIGHTCOMMANDS_H

#include "serialcommand.h"

#include <QByteArray>

namespace EightByEightCommands {

enum FileMode {
    FileMode_Read = 0,
    FileMode_Write = 1,
};

enum Command {
    Command_Format_Filesystem = 0x10,
    Command_Open_File = 0x11,
    Command_Write = 0x12,
    Command_Read = 0x13,
    Command_Close_File = 0x14,
};

QByteArray commandHeader();

/// Format the SPIFFS partition
// TODO: Replace this with a command that just erases the animations?
SerialCommand formatFilesystem();

// Open a file
// Note: fileName less than ? characters
SerialCommand openFile(const QString fileName, FileMode mode);

// Write a block of data to the openend file (open in write mode first)
// Note: data length less than 256 bytes
SerialCommand writeChunk(const QByteArray &data);

// Write data to the opened file (can be arbitrary length)
QList<SerialCommand> write(const QByteArray &data);

// Verify a block of data from the openend file (open in read mode first)
// Note: data length less than 256 bytes
SerialCommand verifyChunk(const QByteArray &data);

// Verify a block of data from the openend file (open in read mode first)
QList<SerialCommand> verify(const QByteArray &data);

// Close the open file
SerialCommand closeFile();

}

#endif // EIGHTBYEIGHTCOMMANDS_H