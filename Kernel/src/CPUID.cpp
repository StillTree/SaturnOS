#include "CPUID.hpp"

#include <cpuid.h>

namespace SaturnKernel {

CPUInfo g_cpuInformation = {};

[[nodiscard]] auto CPUInfo::CPUID(u32 leaf, u32 subleaf) const -> Result<CPUIDResult>
{
	CPUIDResult result {};

	if ((leaf < 0x80000000 && leaf > MaximumLeaf) || (leaf >= 0x80000000 && leaf > MaximumExtendedLeaf))
		return Result<CPUIDResult>::MakeErr(ErrorCode::SerialOutputUnavailabe);

	if (subleaf != 0) {
		__cpuid_count(leaf, subleaf, result.EAX, result.EBX, result.ECX, result.EDX);

		return Result<CPUIDResult>::MakeOk(result);
	}

	__cpuid(leaf, result.EAX, result.EBX, result.ECX, result.EDX);

	return Result<CPUIDResult>::MakeOk(result);
}

auto CPUInfo::SaveInfo() -> Result<void>
{
	auto basicInfo = CPUID(0);
	MaximumLeaf = basicInfo.Value.EAX;

	auto extendedInfo = CPUID(0x80000000);
	MaximumExtendedLeaf = extendedInfo.Value.EAX;

	if (auto features = CPUID(1); features.IsOk()) {
		SupportsAVX = (features.Value.ECX >> 28) != 0;
		SupportsMMX = (features.Value.EDX >> 23) != 0;
		SupportsSSE = (features.Value.EDX >> 25) != 0;
		SupportsSSE2 = (features.Value.EDX >> 26) != 0;
		SupportsXAPIC = (features.Value.EDX >> 9) != 0;
		SupportsX2APIC = (features.Value.ECX >> 21) != 0;
	}

	if (auto features = CPUID(0x80000008); features.IsOk()) {
		PhysicalAddressBits = features.Value.EAX & 0xff;
		VirtualAddressBits = (features.Value.EAX >> 8) & 0xff;
	}

	return Result<void>::MakeOk();
}

}
