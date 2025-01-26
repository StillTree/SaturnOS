#pragma once

#include "Core.h"

typedef struct __attribute__((packed)) IDTEntry {
	u16 AddressLow;
	u16 KernelCS;
	u8 IST;
	u8 Flags;
	u16 AddressMid;
	u32 AddressHigh;
	u32 Reserved;
} IDTEntry;

typedef struct __attribute__((packed)) IDTRegister {
	u16 Size;
	u64 Address;
} IDTRegister;

void SetIDTEntry(u8 vector, u64 handler);
void InitIDT();

/// Executes the `sti` instruction.
inline void EnableInterrupts() { __asm__ volatile("sti"); }

/// Executes the `cli` instruction.
inline void DisableInterrupts() { __asm__ volatile("cli"); }

extern IDTEntry g_idt[256];
