#include "Core.hpp"

namespace SaturnKernel {

inline auto ReadMSR(U32 msr) -> U64
{
	U32 low = 0;
	U32 high = 0;
	__asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));

	return (static_cast<U64>(high) << 32) | low;
}

inline auto WriteMSR(U32 msr, U64 value) -> void
{
	U32 low = static_cast<U32>(value);
	U32 high = static_cast<U32>(value >> 32);

	__asm__ volatile("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

}
