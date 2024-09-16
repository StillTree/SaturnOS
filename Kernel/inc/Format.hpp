#pragma once

#include "Core.hpp"

namespace SaturnKernel
{
	USIZE NumberToHexStringLength(U64 number);
	void NumberToHexString(U64 number, I8* buffer);

	constexpr USIZE MAX_HEX_LENGTH = 16;
}

