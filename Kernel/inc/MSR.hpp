#include "Core.hpp"

namespace SaturnKernel {

inline auto ReadMSR(u32 msr) -> u64
{
	u32 low = 0;
	u32 high = 0;
	__asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));

	return (static_cast<u64>(high) << 32) | low;
}

inline auto WriteMSR(u32 msr, u64 value) -> void
{
	u32 low = static_cast<u32>(value);
	u32 high = static_cast<u32>(value >> 32);

	__asm__ volatile("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

}
