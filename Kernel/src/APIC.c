#include "APIC.h"

#include "ACPI.h"
#include "CPUID.h"
#include "InOut.h"
#include "MSR.h"
#include "Memory/Page.h"

void DisablePIC()
{
	OutputU8(PIC_MASTER_DATA, 0xff);
	OutputU8(PIC_SLAVE_DATA, 0xff);
}

/// Simply returns the base address of the first I/O APIC found.
///
/// TODO: Probably should do some ACPI magic and actually pick the correct one or something.
Reuslt FindIOAPICAddress()
{
	auto madtAddress = g_xsdt->GetACPITableAddress("APIC");
	if (madtAddress.IsError()) {
		return Result<PhysicalAddress>::MakeErr(madtAddress.Error);
	}

	auto* madt = madtAddress.Value.AsPointer<MADT>();

	MADT::BaseEntry* entry = &madt->FirstEntry;
	while (madt->GetAPICEntry(entry)) {
		if (entry->Type == 1) {
			auto* ioEntry = reinterpret_cast<MADT::EntryIO*>(entry);
			return Result<PhysicalAddress>::MakeOk(PhysicalAddress(ioEntry->IOAPICAddress));
		}
	}

	return Result<PhysicalAddress>::MakeErr(ErrorCode::IOAPICNotPresent);
}

volatile u32* g_ioapic = nullptr;

u32 IOAPICReadRegister(u32 reg)
{
	g_ioapic[0] = reg & 0xff;
	return g_ioapic[4];
}

void IOAPICWriteRegister(u32 reg, u32 value)
{
	g_ioapic[0] = reg & 0xff;
	g_ioapic[4] = value;
}

void IOAPICSetRedirectionEntry(u8 irq, u64 entry)
{
	const u32 lowIndex = IOAPIC_REDIRECTION_TABLE_REGISTER_INDEX + (irq * 2);
	const u32 highIndex = IOAPIC_REDIRECTION_TABLE_REGISTER_INDEX + (irq * 2) + 1;

	IOAPICWriteRegister(lowIndex, static_cast<u32>(entry & 0xffffffff));
	IOAPICWriteRegister(highIndex, static_cast<u32>((entry >> 32) & 0xffffffff));
}

Result InitAPIC()
{
	// TODO: Add xAPIC support
	if (!g_cpuInformation.SupportsX2APIC)
		return Result<void>::MakeErr(ErrorCode::X2APICUnsupported);

	// Enables the x2APIC
	u64 apicBase = ReadMSR(LAPIC::BASE_MSR);
	apicBase |= (1 << 11) | (1 << 10);
	WriteMSR(LAPIC::BASE_MSR, apicBase);

	u64 svr = ReadMSR(LAPIC::SVR_REGISTER_MSR);
	WriteMSR(LAPIC::SVR_REGISTER_MSR, svr | 0x100);

	auto ioapicAddress = FindIOAPICAddress();
	if (ioapicAddress.IsError()) {
		return Result<void>::MakeErr(ioapicAddress.Error);
	}

	Page<Size4KiB> ioapicPage(ioapicAddress.Value.Value);
	auto result = ioapicPage.MapTo(
		Frame<Size4KiB>(ioapicAddress.Value), PageTableEntryFlags::Present | PageTableEntryFlags::Writeable | PageTableEntryFlags::NoCache);
	if (result.IsError()) {
		return Result<void>::MakeErr(result.Error);
	}

	g_ioapic = ioapicAddress.Value.AsRawPointer<u32>();

	// Set the PS/2 keyboard handler
	IOAPIC::SetRedirectionEntry(1, 33);

	return Result<void>::MakeOk();
}

void EOISignal() { WriteMSR(LAPIC_SVC_REGISTER_EOI, 0); }
