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
Result FindIOAPICAddress(PhysicalAddress* address)
{
	PhysicalAddress madtAddress;
	Result result = GetACPITableAddress("APIC", &madtAddress);
	if (result) {
		return result;
	}

	MADT* madt = PhysicalAddressAsPointer(madtAddress);

	MADTBaseEntry* entry = madt->Entries;
	while (MADTGetAPICEntry(madt, &entry)) {
		if (entry->Type == 1) {
			MADTEntryIO* ioEntry = (MADTEntryIO*)entry;
			*address = ioEntry->IOAPICAddress;
			return ResultOk;
		}
	}

	return ResultIOAPICNotPresent;
}

u32* g_ioapic = nullptr;

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

	IOAPICWriteRegister(lowIndex, (u32)(entry & 0xffffffff));
	IOAPICWriteRegister(highIndex, (u32)((entry >> 32) & 0xffffffff));
}

Result InitAPIC()
{
	// TODO: Add xAPIC support
	if (!g_cpuInformation.SupportsX2APIC)
		return ResultX2APICUnsupported;

	// Enables the x2APIC
	u64 apicBase = ReadMSR(LAPIC_BASE_MSR);
	apicBase |= (1 << 11) | (1 << 10);
	WriteMSR(LAPIC_BASE_MSR, apicBase);

	u64 svr = ReadMSR(LAPIC_SVR_REGISTER_MSR);
	WriteMSR(LAPIC_SVR_REGISTER_MSR, svr | 0x100);

	PhysicalAddress ioapicAddress;
	Result result = FindIOAPICAddress(&ioapicAddress);
	if (result) {
		return result;
	}

	Page4KiB ioapicPage = ioapicAddress;
	result = Page4KiBMapTo(ioapicPage, ioapicAddress, Present | Writeable | NoCache);
	if (result) {
		return result;
	}

	g_ioapic = (u32*)ioapicAddress;

	// Set the PS/2 keyboard handler
	IOAPICSetRedirectionEntry(1, 33);

	return ResultOk;
}

void EOISignal() { WriteMSR(LAPIC_SVC_REGISTER_EOI, 0); }
