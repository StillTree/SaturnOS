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
} CPUInfo;

/// A wrapper around the CPUID macros from GCC with some error checks.
Result CPUID(u32 leaf, u32 subleaf, CPUIDResult* result);

/// Fills in the whole structure with actual data.
Result CPUIDSaveInfo(CPUInfo* cpuInfo);

extern CPUInfo g_cpuInformation;
