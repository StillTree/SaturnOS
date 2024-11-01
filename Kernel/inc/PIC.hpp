#pragma once

#include "Core.hpp"

namespace SaturnKernel {

constexpr U8 PIC_OFFSET = 32;

constexpr U8 PIC_MASTER_COMMAND = 0x20;
constexpr U8 PIC_MASTER_DATA = 0x21;
constexpr U8 PIC_SLAVE_COMMAND = 0xa0;
constexpr U8 PIC_SLAVE_DATA = 0xa1;

constexpr U8 PIC_EOI_SIGNAL = 0x20;

enum class IRQMask : U16 {
	Timer = 1,
	Keyboard = 1 << 1,
	// I'm probably not gonna be using the rest of them
	Mouse = 1 << 12
};

inline auto operator|(IRQMask a, IRQMask b) -> IRQMask { return static_cast<IRQMask>(static_cast<U16>(a) | static_cast<U16>(b)); }

auto EOISignal(U8 interruptVector) -> void;
auto ReinitializePIC() -> void;

}
