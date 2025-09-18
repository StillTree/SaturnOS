#include "CPUInfo.h"

#include <cpuid.h>

CPUInfo g_cpuInformation = {};

Result CPUID(const CPUInfo* cpuInfo, u32 leaf, u32 subleaf, CPUIDResult* result)
{
	if ((leaf < 0x80000000 && leaf > cpuInfo->MaximumLeaf) || (leaf >= 0x80000000 && leaf > cpuInfo->MaximumExtendedLeaf))
		return ResultSerialOutputUnavailable;

	if (subleaf != 0) {
		__cpuid_count(leaf, subleaf, result->EAX, result->EBX, result->ECX, result->EDX);

		return ResultOk;
	}

	__cpuid(leaf, result->EAX, result->EBX, result->ECX, result->EDX);

	return ResultOk;
}

Result CPUIDSaveInfo(CPUInfo* cpuInfo)
{
	CPUIDResult basicInfo = {};
	Result result = CPUID(cpuInfo, 0, 0, &basicInfo);
	cpuInfo->MaximumLeaf = basicInfo.EAX;

	CPUIDResult extendedInfo = {};
	result = CPUID(cpuInfo, 0x80000000, 0, &extendedInfo);
	cpuInfo->MaximumExtendedLeaf = extendedInfo.EAX;

	CPUIDResult featuresInfo = {};
	result = CPUID(cpuInfo, 1, 0, &featuresInfo);
	if (!result) {
		cpuInfo->SupportsAVX = (featuresInfo.ECX & (1U << 28)) != 0;
		cpuInfo->SupportsX2APIC = (featuresInfo.ECX & (1U << 21)) != 0;
		cpuInfo->SupportsRDRAND = (featuresInfo.ECX & (1U << 30)) != 0;
		cpuInfo->SupportsMMX = (featuresInfo.EDX & (1U << 23)) != 0;
		cpuInfo->SupportsSSE = (featuresInfo.EDX & (1U << 25)) != 0;
		cpuInfo->SupportsSSE2 = (featuresInfo.EDX & (1U << 26)) != 0;
		cpuInfo->SupportsXAPIC = (featuresInfo.EDX & (1U << 9)) != 0;
	}

	result = CPUID(cpuInfo, 0x80000008, 0, &featuresInfo);
	if (!result) {
		cpuInfo->PhysAddrBits = featuresInfo.EAX & 0xff;
		cpuInfo->VirtAddrBits = (featuresInfo.EAX >> 8) & 0xff;
	}

	result = CPUID(cpuInfo, 7, 0, &featuresInfo);
	if (!result) {
		cpuInfo->SupportsRDSEED = (featuresInfo.EBX & (1U << 18)) != 0;
	}

	return ResultOk;
}
