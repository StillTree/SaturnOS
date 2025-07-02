#include "ACPI.h"

#include "Memory.h"

XSDT* g_xsdt = nullptr;

bool SDTIsChecksumValid(const SDTHeader* acpiTable)
{
	usz sum = 0;
	u8* bytes = (u8*)acpiTable;

	for (usz i = 0; i < acpiTable->Length; i++) {
		sum += bytes[i];
	}

	return (sum & 0xff) == 0;
}

static usz XSDTEntries(const XSDT* acpiTable) { return (acpiTable->Header.Length - sizeof(SDTHeader)) / 8; }

Result GetACPITableAddress(const i8* signature, PhysicalAddress* address)
{
	for (usz i = 0; i < XSDTEntries(g_xsdt); i++) {
		SDTHeader* table = PhysicalAddressAsPointer(g_xsdt->Entries[i]);

		if (SDTIsChecksumValid(table) && MemoryCompare(signature, (i8*)table->Signature, 4)) {
			*address = g_xsdt->Entries[i];
			return ResultOk;
		}
	}

	return ResultInvalidSDTSignature;
}

usz MCFGEntries(const MCFG* mcfg) { return (mcfg->Header.Length - sizeof(SDTHeader) - 8) / sizeof(MCFGEntry); }

u8 MADTGetAPICEntry(const MADT* madt, MADTBaseEntry** pointer)
{
	u8* offset = (u8*)*pointer;

	if (offset + (*pointer)->Length >= (u8*)(madt) + madt->Header.Length) {
		return 0;
	}

	offset += (*pointer)->Length;
	*pointer = (MADTBaseEntry*)offset;

	return 1;
}

Result InitXSDT()
{
	XSDT* xsdt = PhysicalAddressAsPointer(g_bootInfo.XSDTPhysicalAddress);

	if (!SDTIsChecksumValid((SDTHeader*)xsdt)) {
		return ResultXSDTCorrupted;
	}

	g_xsdt = xsdt;

	return ResultOk;
}
