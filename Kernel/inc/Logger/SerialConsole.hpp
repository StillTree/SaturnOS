#pragma once

#include "Core.hpp"

namespace SaturnKernel
{
	struct SerialConsoleLogger
	{
		void Init(U16 port);
		void WriteChar(U8 character) const;
		void WriteString(const I8* string) const;

		U16 Port;
	};
}
