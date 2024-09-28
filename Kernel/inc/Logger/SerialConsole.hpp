#pragma once

#include "Core.hpp"

namespace SaturnKernel
{
	struct SerialConsoleLogger
	{
		void Init(U16 port);
		void WriteChar(U8 character);
		void WriteString(const I8* string);

		U16 port;
	};
}
