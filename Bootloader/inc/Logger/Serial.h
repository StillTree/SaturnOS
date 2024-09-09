#pragma once

#include "Uefi.h"

/// Serial logger's "state".
typedef struct SerialLoggerData
{
	UINT16 port;
} SerialLoggerData;

/// Initializes the serial output device at port 0x3f8.
EFI_STATUS InitSerialLogger(EFI_SYSTEM_TABLE* systemTable, SerialLoggerData* logger);
/// Writes a character to the serial output device,
/// if the character exceeds the extended ASCII range "?" gets printed as a fallback.
VOID SerialLoggerWriteChar(SerialLoggerData* logger, CHAR16 character);
/// Writes an entire string to the serial output device.
///
/// Note: The string has to be NULL-terminated.
VOID SerialLoggerWriteString(SerialLoggerData* logger, CHAR16* string);

