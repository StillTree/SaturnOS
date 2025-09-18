#pragma once

#include "Core.h"

static inline u64 ReadTSC()
{
	u32 low = 0;
	u32 high = 0;
	__asm__ volatile("rdtsc" : "=a"(low), "=d"(high));

	return ((u64)high << 32) | low;
}

static inline u64 ReadMSR(u32 msr)
{
	u32 low = 0;
	u32 high = 0;
	__asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));

	return ((u64)high << 32) | low;
}

static inline void WriteMSR(u32 msr, u64 value)
{
	u32 low = (u32)value;
	u32 high = (u32)(value >> 32);

	__asm__ volatile("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

static inline void OutU8(u16 port, u8 value) { __asm__ volatile("outb %b0, %w1" : : "a"(value), "Nd"(port) : "memory"); }

static inline u8 InU8(u16 port)
{
	u8 result = -1;
	__asm__ volatile("inb %w1, %b0" : "=a"(result) : "Nd"(port) : "memory");

	return result;
}

static inline void IOWait() { OutU8(0x80, 0); }

static inline bool RDSEED(u64* value)
{
	bool success = false;

	for (usz i = 0; i < 10; ++i) {
		__asm__ volatile("rdseed %0\n"
						 "setc %1"
			: "=r"(*value), "=qm"(success));

		if (success) {
			return true;
		}
	}

	return false;
}
