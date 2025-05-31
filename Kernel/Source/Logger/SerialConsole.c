#include "Logger/SerialConsole.h"

#include "InOut.h"

Result SerialConsoleInit(SerialConsoleLogger* logger, u16 port)
{
	logger->Port = port;

	OutputU8(logger->Port + 1, 0x00);
	OutputU8(logger->Port + 3, 0x80);
	OutputU8(logger->Port, 0x03);
	OutputU8(logger->Port + 1, 0x00);
	OutputU8(logger->Port + 3, 0x03);
	OutputU8(logger->Port + 2, 0xc7);
	OutputU8(logger->Port + 4, 0x0b);
	// Set in loopback mode
	OutputU8(logger->Port + 4, 0x1e);

	OutputU8(logger->Port, 0xae);

	// If we didn't get back the exact same byte that we sent in loopback mode,
	// the device is not functioning corretly and should not be used
	if (InputU8(logger->Port + 0) != 0xae) {
		return ResultSerialOutputUnavailable;
	}

	// If it is functioning correctly we set it in normal operation mode
	OutputU8(logger->Port + 4, 0x0f);

	return ResultOk;
}

void SerialConsoleWriteChar(SerialConsoleLogger* logger, u8 character)
{
	if (character > 126) {
		character = '?';
	}

	OutputU8(logger->Port, character);
}

void SerialConsoleWriteString(SerialConsoleLogger* logger, const i8* string)
{
	usz i = 0;
	while (string[i]) {
		SerialConsoleWriteChar(logger, string[i++]);
	}
}
