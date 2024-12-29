#pragma once

#include "Core.hpp"
#include "Result.hpp"

namespace SaturnKernel {

struct CPUIDResult {
	U32 EAX;
	U32 EBX;
	U32 ECX;
	U32 EDX;
};

struct CPUInfo {
	U32 MaximumLeaf = 0;
	U32 MaximumExtendedLeaf = 0x80000000;

	bool SupportsSSE = false;
	bool SupportsSSE2 = false;
	bool SupportsMMX = false;
	bool SupportsAVX = false;

	bool SupportsXAPIC = false; // Or just APIC
	bool SupportsX2APIC = false;

	U8 PhysicalAddressBits = 0;
	U8 VirtualAddressBits = 0;

	/// A wrapper around the CPUID macros from GCC with some error checks.
	[[nodiscard]] auto CPUID(U32 leaf, U32 subleaf = 0) const -> Result<CPUIDResult>;

	/// Fills in the whole structure with actual data.
	auto SaveInfo() -> Result<void>;
};

extern CPUInfo g_cpuInformation;

}
