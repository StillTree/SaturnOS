#pragma once

#include "Core.hpp"
#include "Result.hpp"

namespace SaturnKernel
{
	struct SerialConsoleLogger
	{
		auto Init(U16 port) -> Result<void>;
		auto WriteChar(U8 character) const -> void;
		auto WriteString(const I8* string) const -> void;

		U16 Port;
	};
}
