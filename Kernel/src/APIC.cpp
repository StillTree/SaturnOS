#include "APIC.hpp"

#include "ACPI.hpp"
#include "CPUID.hpp"
#include "InOut.hpp"
#include "MSR.hpp"

namespace SaturnKernel {

auto DisablePIC() -> void
{
	OutputU8(PIC_MASTER_DATA, 0xff);
	OutputU8(PIC_SLAVE_DATA, 0xff);
}

namespace {
	/// Simply returns the base address of the first I/O APIC found.
	///
	/// TODO: Probably should do some ACPI magic and actually pick the correct one or something.
	auto FindIOAPICAddress() -> Result<PhysicalAddress>
	{
		auto madtAddress = g_xsdt->GetACPITableAddress("APIC");
		if (madtAddress.IsError()) {
			return Result<PhysicalAddress>::MakeErr(madtAddress.Error);
		}

		auto* madt = madtAddress.Value.AsPointer<MADT>();

		MADT::EntryHeader* entry = &madt->FirstEntry;
		while (madt->GetAPICEntry(entry)) {
			if (entry->Type == 1) {
				auto* ioEntry = reinterpret_cast<MADT::EntryIO*>(entry);
				return Result<PhysicalAddress>::MakeOk(PhysicalAddress(ioEntry->IOAPICAddress));
			}
		}

		return Result<PhysicalAddress>::MakeErr(ErrorCode::IOAPICNotPresent);
	}

	volatile U32* g_ioapic = nullptr;
}

namespace IOAPIC {
	auto ReadRegister(U32 reg) -> U32
	{
		g_ioapic[0] = reg & 0xff;
		return g_ioapic[4];
	}

	auto WriteRegister(U32 reg, U32 value) -> void
	{
		g_ioapic[0] = reg & 0xff;
		g_ioapic[4] = value;
	}
}

auto InitAPIC() -> Result<void>
{
	if (!g_cpuInformation.SupportsX2APIC)
		return Result<void>::MakeErr(ErrorCode::X2APICUnsupported);

	// Enables the x2APIC
	U64 apicBase = ReadMSR(LAPIC::BASE_MSR);
	apicBase |= (1 << 11) | (1 << 10);
	WriteMSR(LAPIC::BASE_MSR, apicBase);

	U64 svr = ReadMSR(LAPIC::SVR_REGISTER_MSR);
	WriteMSR(LAPIC::SVR_REGISTER_MSR, svr | 0x100);

	auto ioapicAddress = FindIOAPICAddress();
	if (ioapicAddress.IsError()) {
		return Result<void>::MakeErr(ioapicAddress.Error);
	}

	// TODO: Map the address
	g_ioapic = ioapicAddress.Value.AsPointer<U32>();

	// Set the PS/2 keyboard handler
	IOAPIC::WriteRegister(IOAPIC::REDIRECTION_TABLE_REGISTER_INDEX + (1 * 2), 33);
	IOAPIC::WriteRegister(IOAPIC::REDIRECTION_TABLE_REGISTER_INDEX + (1 * 2) + 1, 0);

	return Result<void>::MakeOk();
}

auto EOISignal() -> void { WriteMSR(0x80b, 0); }

}
