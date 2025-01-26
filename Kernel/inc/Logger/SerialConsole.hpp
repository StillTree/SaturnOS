#pragma once

#include "Core.hpp"
#include "Result.hpp"

namespace SaturnKernel {

struct SerialConsoleLogger {
	auto Init(u16 port) -> Result<void>;
	auto WriteChar(u8 character) const -> void;
	auto WriteString(const i8* string) const -> void;

	u16 Port;
};

}
