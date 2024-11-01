#include "Logger/SerialConsole.hpp"

#include "InOut.hpp"

namespace SaturnKernel {

auto SerialConsoleLogger::Init(U16 port) -> Result<void>
{
	this->Port = port;

	OutputU8(this->Port + 1, 0x00);
	OutputU8(this->Port + 3, 0x80);
	OutputU8(this->Port, 0x03);
	OutputU8(this->Port + 1, 0x00);
	OutputU8(this->Port + 3, 0x03);
	OutputU8(this->Port + 2, 0xc7);
	OutputU8(this->Port + 4, 0x0b);
	// Set in loopback mode
	OutputU8(this->Port + 4, 0x1e);

	OutputU8(this->Port, 0xae);

	// If we didn't get back the exact same byte that we sent in loopback mode,
	// the device is not functioning corretly and should not be used
	if (InputU8(this->Port + 0) != 0xae) {
		return Result<void>::MakeErr(ErrorCode::SerialOutputUnavailabe);
	}

	// If it is functioning correctly we set it in normal operation mode
	OutputU8(this->Port + 4, 0x0f);

	return Result<void>::MakeOk();
}

auto SerialConsoleLogger::WriteChar(U8 character) const -> void
{
	if (character > 126) {
		character = '?';
	}

	OutputU8(this->Port, character);
}

auto SerialConsoleLogger::WriteString(const I8* string) const -> void
{
	USIZE i = 0;
	while (string[i]) {
		WriteChar(string[i++]);
	}
}

}
