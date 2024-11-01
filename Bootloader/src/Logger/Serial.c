#include "Logger/Serial.h"

static inline VOID OutputByte(UINT16 port, UINT8 value) { __asm__ volatile("outb %b0, %w1" : : "a"(value), "Nd"(port) : "memory"); }

static inline UINT8 InputByte(UINT16 port)
{
	UINT8 result;
	__asm__ volatile("inb %w1, %b0" : "=a"(result) : "Nd"(port) : "memory");

	return result;
}

EFI_STATUS InitSerialLogger(EFI_SYSTEM_TABLE* systemTable, SerialLoggerData* logger)
{
	systemTable->ConOut->OutputString(systemTable->ConOut, L"Initializing the serial output device logger... ");

	logger->port = 0x3f8;

	OutputByte(logger->port + 1, 0x00);
	OutputByte(logger->port + 3, 0x80);
	OutputByte(logger->port, 0x03);
	OutputByte(logger->port + 1, 0x00);
	OutputByte(logger->port + 3, 0x03);
	OutputByte(logger->port + 2, 0xC7);
	OutputByte(logger->port + 4, 0x0B);
	// Set in loopback mode
	OutputByte(logger->port + 4, 0x1E);

	OutputByte(logger->port, 0xae);

	// If we didn't get back the exact same byte that we sent in loopback mode,
	// the device is not functioning corretly and should not be used
	if (InputByte(logger->port + 0) != 0xAE) {
		systemTable->ConOut->OutputString(systemTable->ConOut, L"The serial output device is not functioning correctly\r\n");

		return EFI_DEVICE_ERROR;
	}

	// If it is functioning correctly we set it in normal operation mode
	OutputByte(logger->port + 4, 0x0F);

	systemTable->ConOut->OutputString(systemTable->ConOut, L"Done\r\n");

	return EFI_SUCCESS;
}

VOID SerialLoggerWriteChar(SerialLoggerData* logger, CHAR16 character)
{
	if (character > 255) {
		character = L'?';
	}

	OutputByte(logger->port, character);
}

VOID SerialLoggerWriteString(SerialLoggerData* logger, CHAR16* string)
{
	UINTN i = 0;
	while (string[i]) {
		SerialLoggerWriteChar(logger, string[i++]);
	}
}
