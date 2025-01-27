#include "Core.h"

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
