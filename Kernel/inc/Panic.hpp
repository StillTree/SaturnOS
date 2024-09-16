#pragma once

#include "Core.hpp"

namespace SaturnKernel
{
	[[noreturn]]
	void Hang();
	[[noreturn]]
	void Panic(const I8* message);
}

