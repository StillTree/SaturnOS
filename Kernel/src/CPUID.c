#include "CPUID.h"

#include <cpuid.h>

CPUInfo g_cpuInformation = {};

Result CPUID(CPUInfo* cpuInfo, u32 leaf, u32 subleaf, CPUIDResult* result)
{
	if ((leaf < 0x80000000 && leaf > cpuInfo->MaximumLeaf) || (leaf >= 0x80000000 && leaf > cpuInfo->MaximumExtendedLeaf))
		return ResultSerialOutputUnavailabe;

	if (subleaf != 0) {
		__cpuid_count(leaf, subleaf, result->EAX, result->EBX, result->ECX, result->EDX);

		return ResultOk;
	}

	__cpuid(leaf, result->EAX, result->EBX, result->ECX, result->EDX);

	return ResultOk;
}

Result CPUIDSaveInfo(CPUInfo* cpuInfo)
{
	CPUIDResult basicInfo {};
	Result result = CPUID(cpuInfo, 0, 0, &basicInfo);
	cpuInfo->MaximumLeaf = basicInfo.EAX;

	CPUIDResult extendedInfo {};
	result = CPUID(cpuInfo, 0x80000000, 0, &extendedInfo);
	cpuInfo->MaximumExtendedLeaf = extendedInfo.EAX;

	CPUIDResult featuresInfo = {};
	result = CPUID(cpuInfo, 1, 0, &featuresInfo);
	if (!result) {
		cpuInfo->SupportsAVX = (features.ECX >> 28) != 0;
		cpuInfo->SupportsMMX = (features.EDX >> 23) != 0;
		cpuInfo->SupportsSSE = (features.EDX >> 25) != 0;
		cpuInfo->SupportsSSE2 = (features.EDX >> 26) != 0;
		cpuInfo->SupportsXAPIC = (features.EDX >> 9) != 0;
		cpuInfo->SupportsX2APIC = (features.ECX >> 21) != 0;
	}

	result = CPUID(cpuInfo, 0x80000008, 0, &featuresInfo);
	if (!result) {
		PhysicalAddressBits = features.EAX & 0xff;
		VirtualAddressBits = (features.EAX >> 8) & 0xff;
	}

	return ResultOk;
}
