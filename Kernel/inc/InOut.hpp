#pragma once

#include "Core.hpp"

namespace SaturnKernel
{
	void OutputU8(U16 port, U8 value);
	auto InputU8(U16 port) -> U8;
	void IOWait();
}
