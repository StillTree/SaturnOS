#pragma once

#include "Core.hpp"

namespace SaturnKernel {

struct __attribute__((packed)) IDTEntry {
	u16 AddressLow;
	u16 KernelCS;
	u8 IST;
	u8 Flags;
	u16 AddressMid;
	u32 AddressHigh;
	u32 Reserved;
};

struct __attribute__((packed)) IDTRegister {
	u16 Size;
	u64 Address;
};

auto SetIDTEntry(u8 vector, u64 handler) -> void;
auto InitIDT() -> void;

/// Executes the `sti` instruction.
inline auto EnableInterrupts() -> void { __asm__ volatile("sti"); }

/// Executes the `cli` instruction.
inline auto DisableInterrupts() -> void { __asm__ volatile("cli"); }

extern IDTEntry g_idt[256];

}
