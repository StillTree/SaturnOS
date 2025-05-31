#include "APIC.h"

#include "ACPI.h"
#include "CPUInfo.h"
#include "InOut.h"
#include "Logger.h"
#include "MSR.h"
#include "Memory/Page.h"
#include "Memory/VirtualMemoryAllocator.h"

void DisablePIC()
{
	OutputU8(PIC_MASTER_DATA, 0xff);
	OutputU8(PIC_SLAVE_DATA, 0xff);
}

APIC g_apic;

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
		if (entry->Type == MADTEntryIOAPIC) {
			MADTEntryIO* ioEntry = (MADTEntryIO*)entry;
			*address = ioEntry->IOAPICAddress;
			return ResultOk;
		}
	}

	return ResultIOAPICNotPresent;
}

u32 LAPICReadRegister(u32 reg)
{
	if (g_apic.X2APICMode) {
		return ReadMSR(MSR_X2APIC_BASE + (reg >> 4));
	}

	// xAPIC mode
	return *(g_apic.XAPICAddress + (reg / 4));
}

void LAPICWriteRegister(u32 reg, u32 value)
{
	if (g_apic.X2APICMode) {
		WriteMSR(MSR_X2APIC_BASE + (reg >> 4), value);
		return;
	}

	// xAPIC mode
	*(g_apic.XAPICAddress + (reg / 4)) = value;
}

u32 IOAPICReadRegister(u32 reg)
{
	g_apic.IOAPICAddress[0] = reg & 0xff;
	return g_apic.IOAPICAddress[4];
}

void IOAPICWriteRegister(u32 reg, u32 value)
{
	g_apic.IOAPICAddress[0] = reg & 0xff;
	g_apic.IOAPICAddress[4] = value;
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
	if (!g_cpuInformation.SupportsX2APIC) {
		SK_LOG_DEBUG("x2APIC not supported, falling back to xAPIC");

		// Enabled the xAPIC (or just APIC, I guess)
		u64 apicBase = ReadMSR(MSR_APIC_BASE);
		apicBase |= (1 << 11);
		WriteMSR(MSR_APIC_BASE, apicBase);

		Frame4KiB xapicFrame = apicBase & ~0xfff;
		g_apic.X2APICMode = false;

		Page4KiB xapicAddress;
		Result result = AllocateMMIORegion(&g_virtualMemoryAllocator, xapicFrame, 0x1000, PageWriteable | PageNoCache, &xapicAddress);
		if (result) {
			return result;
		}

		g_apic.XAPICAddress = (u32*)xapicAddress;
	} else {
		// Enables the x2APIC
		u64 apicBase = ReadMSR(MSR_APIC_BASE);
		apicBase |= (1 << 11) | (1 << 10);
		WriteMSR(MSR_APIC_BASE, apicBase);

		g_apic.X2APICMode = true;
		g_apic.XAPICAddress = nullptr;
	}

	LAPICWriteRegister(LAPIC_SVR_REGISTER, 0x1ff);

	PhysicalAddress ioapicBase;
	Result result = FindIOAPICAddress(&ioapicBase);
	if (result) {
		return result;
	}

	Page4KiB ioapicAddress;
	result = AllocateMMIORegion(&g_virtualMemoryAllocator, ioapicBase & ~0xfff, 0x1000, PageWriteable | PageNoCache, &ioapicAddress);
	if (result) {
		return result;
	}

	g_apic.IOAPICAddress = (u32*)(ioapicAddress + (ioapicBase & 0xfff));

	InitAPICTimer();

	// Set the PS/2 keyboard handler
	IOAPICSetRedirectionEntry(1, 33);

	return ResultOk;
}

void EOISignal() { LAPICWriteRegister(LAPIC_EOI_REGISTER, 0); }

void InitAPICTimer()
{
	u16 pitDivisor = 1193182 / 100;

	// Disable, configure and enable for calibrating
	LAPICWriteRegister(LAPIC_TIMER_INITIAL_REGISTER, 0);
	LAPICWriteRegister(LAPIC_TIMER_DIVISOR_REGISTER, 0xb);
	LAPICWriteRegister(LAPIC_LVT_TIMER_REGISTER, 0x10000);
	LAPICWriteRegister(LAPIC_TIMER_INITIAL_REGISTER, 0xffffffff);

	u32 initialCount = LAPICReadRegister(LAPIC_TIMER_CURRENT_REGISTER);

	// Set the divisor to 100 MHz
	OutputU8(0x43, 0x30);
	OutputU8(0x40, pitDivisor & 0xff);
	OutputU8(0x40, pitDivisor >> 8);

	// Credit to, Omar Elghoul, the luxOS author for this algorithm that I stole from him 💀
	// https://github.com/lux-operating-system/kernel/blob/main/src/platform/x86_64/apic/timer.c#L61
	u16 currentCounter = pitDivisor;
	u16 oldCurrentCounter = pitDivisor;

	while ((currentCounter <= oldCurrentCounter) && currentCounter) {
		oldCurrentCounter = currentCounter;

		OutputU8(0x43, 0);
		currentCounter = (u16)InputU8(0x40);
		currentCounter |= (u16)InputU8(0x40) << 8;
	}

	u32 finalCount = LAPICReadRegister(LAPIC_TIMER_CURRENT_REGISTER);

	// Disable the LAPIC timer
	LAPICWriteRegister(LAPIC_TIMER_INITIAL_REGISTER, 0);
	g_apic.LAPICTimerFrequency = (u64)(initialCount - finalCount) * 100;

	// Disable the PIT (setting an invalid divisor)
	// I don't know if this even does somthing, but hey, that won't hurt (hopefully...)
	OutputU8(0x43, 0x30);
	OutputU8(0x40, 0);
	OutputU8(0x40, 0);

	SK_LOG_DEBUG("LAPIC Timer frequency: %u MHz", g_apic.LAPICTimerFrequency / 1000000);

	LAPICWriteRegister(LAPIC_LVT_TIMER_REGISTER, ((1 << 17) & ~(1 << 18)) | 34);
	LAPICWriteRegister(LAPIC_TIMER_INITIAL_REGISTER, g_apic.LAPICTimerFrequency / 100);
}
