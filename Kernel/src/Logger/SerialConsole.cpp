#include "Logger/SerialConsole.hpp"

#include "InOut.hpp"

namespace SaturnKernel
{
	void SerialConsoleLogger::Init(U16 port)
	{
		this->port = port;

		OutputU8(this->port + 1, 0x00);
		OutputU8(this->port + 3, 0x80);
		OutputU8(this->port, 0x03);
		OutputU8(this->port + 1, 0x00);
		OutputU8(this->port + 3, 0x03);
		OutputU8(this->port + 2, 0xc7);
		OutputU8(this->port + 4, 0x0b);
		// Set in loopback mode
		OutputU8(this->port + 4, 0x1e);

		OutputU8(this->port, 0xae);

		// If we didn't get back the exact same byte that we sent in loopback mode,
		// the device is not functioning corretly and should not be used
		if(InputU8(this->port + 0) != 0xae)
		{
			// TODO: Some sort of error handling through return types
			return;
		}

		// If it is functioning correctly we set it in normal operation mode
		OutputU8(this->port + 4, 0x0f);
	}

	void SerialConsoleLogger::WriteChar(U8 character)
	{
		if(character > 126)
		{
			character = '?';
		}

		OutputU8(this->port, character);
	}

	void SerialConsoleLogger::WriteString(const I8* string)
	{
		USIZE i = 0;
		while(string[i])
		{
			WriteChar(string[i++]);
		}
	}
}
