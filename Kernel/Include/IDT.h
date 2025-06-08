#pragma once

#include "Core.h"

typedef enum IDTEntryFlags : u8 {
	/// Clears the interrupt flag before running the handler function.
	IDTEntryInterruptGate = 14,
	/// Does not clear the interrupt flag before running the handler function.
	IDTEntryTrapGate = 15,
	/// Minimum ring to trigger this interrupt, otherwise #GP fault.
	IDTEntryDPL0 = 0 << 5,
	/// Minimum ring to trigger this interrupt, otherwise #GP fault.
	IDTEntryDPL3 = 3 << 5,
	/// The entry is valid.
	IDTEntryPresent = 1 << 7
} IDTEntryFlags;

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

void SetIDTEntry(u8 vector, u64 handlerFn, u8 flags, u8 istNumber);
void InitIDT();

/// Executes the `sti` instruction.
static inline void EnableInterrupts() { __asm__ volatile("sti"); }

/// Executes the `cli` instruction.
static inline void DisableInterrupts() { __asm__ volatile("cli"); }

extern IDTEntry g_idt[256];
