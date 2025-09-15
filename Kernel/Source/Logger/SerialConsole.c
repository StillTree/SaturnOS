#include "Logger/SerialConsole.h"

#include "Instructions.h"

Result SerialConsoleInit(SerialConsoleLogger* logger, u16 port)
{
	logger->Port = port;

	OutU8(logger->Port + 1, 0x00);
	OutU8(logger->Port + 3, 0x80);
	OutU8(logger->Port, 0x03);
	OutU8(logger->Port + 1, 0x00);
	OutU8(logger->Port + 3, 0x03);
	OutU8(logger->Port + 2, 0xc7);
	OutU8(logger->Port + 4, 0x0b);
	// Set in loopback mode
	OutU8(logger->Port + 4, 0x1e);

	OutU8(logger->Port, 0xae);

	// If we didn't get back the exact same byte that we sent in loopback mode,
	// the device is not functioning corretly and should not be used
	if (InU8(logger->Port + 0) != 0xae) {
		return ResultSerialOutputUnavailable;
	}

	// If it is functioning correctly we set it in normal operation mode
	OutU8(logger->Port + 4, 0x0f);

	return ResultOk;
}

void SerialConsoleWriteChar(SerialConsoleLogger* logger, u8 character)
{
	if (character > 126) {
		character = '?';
	}

	OutU8(logger->Port, character);
}

void SerialConsoleWriteString(SerialConsoleLogger* logger, const i8* string)
{
	usz i = 0;
	while (string[i]) {
		SerialConsoleWriteChar(logger, string[i++]);
	}
}
