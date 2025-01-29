#pragma once

#include "Core.h"
#include "Result.h"

typedef struct CPUIDResult {
	u32 EAX;
	u32 EBX;
	u32 ECX;
	u32 EDX;
} CPUIDResult;

typedef struct CPUInfo {
	u32 MaximumLeaf;
	u32 MaximumExtendedLeaf;

	bool SupportsSSE;
	bool SupportsSSE2;
	bool SupportsMMX;
	bool SupportsAVX;

	bool SupportsXAPIC; // Or just APIC
	bool SupportsX2APIC;

	u8 PhysicalAddressBits;
	u8 VirtualAddressBits;
} CPUInfo;

/// A wrapper around the CPUID macros from GCC with some error checks.
Result CPUID(const CPUInfo* cpuInfo, u32 leaf, u32 subleaf, CPUIDResult* result);

/// Fills in the whole structure with actual data.
Result CPUIDSaveInfo(CPUInfo* cpuInfo);

extern CPUInfo g_cpuInformation;
