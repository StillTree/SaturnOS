#include "APIC.hpp"

#include "InOut.hpp"
#include "Logger.hpp"
#include "MSR.hpp"

namespace SaturnKernel {

auto InitAPIC() -> Result<void>
{
	// Disabling the 8259 PIC
	OutputU8(PIC_MASTER_DATA, 0xff);
	OutputU8(PIC_SLAVE_DATA, 0xff);

	U64 apicBase = ReadMSR(IA32_APIC_BASE_MSR);
	apicBase |= (1 << 11);
	WriteMSR(IA32_APIC_BASE_MSR, apicBase);

	SK_LOG_DEBUG("Local APIC base address: {}", apicBase & 0xfffff000);

	return Result<void>::MakeOk();
}

}
