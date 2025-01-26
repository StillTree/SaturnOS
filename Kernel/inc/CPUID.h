#pragma once

#include "Core.h"
#include "Result.h"

namespace SaturnKernel {

struct CPUIDResult {
	u32 EAX;
	u32 EBX;
	u32 ECX;
	u32 EDX;
};

struct CPUInfo {
	u32 MaximumLeaf = 0;
	u32 MaximumExtendedLeaf = 0x80000000;

	bool SupportsSSE = false;
	bool SupportsSSE2 = false;
	bool SupportsMMX = false;
	bool SupportsAVX = false;

	bool SupportsXAPIC = false; // Or just APIC
	bool SupportsX2APIC = false;

	u8 PhysicalAddressBits = 0;
	u8 VirtualAddressBits = 0;

	/// A wrapper around the CPUID macros from GCC with some error checks.
	[[nodiscard]] auto CPUID(u32 leaf, u32 subleaf = 0) const -> Result<CPUIDResult>;

	/// Fills in the whole structure with actual data.
	auto SaveInfo() -> Result<void>;
};

extern CPUInfo g_cpuInformation;

}
