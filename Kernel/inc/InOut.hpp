#pragma once

#include "Core.hpp"

namespace SaturnKernel
{
	auto OutputU8(U16 port, U8 value) -> void;
	auto InputU8(U16 port) -> U8;
	auto IOWait() -> void;
}
