#include "Memory/VirtualAddress.h"

Result VirtualAddressToPhysical(VirtualAddress address, const PageTableEntry* p4Table, PhysicalAddress* physicalAddress)
{
	u16 p4Index = VirtualAddressPage4Index(address);

	// If there is no Level 3 table at the expected level 4's index, the address's page is unmapped.
	if (!(p4Table[p4Index] & PagePresent)) {
		return ResultSerialOutputUnavailabe;
	}

	u16 p3Index = VirtualAddressPage3Index(address);
	PageTableEntry* p3Table = PhysicalAddressAsPointer(p4Table[p4Index] & ~(0xfff));

	// If there is no Level 2 table at the expected level 3's index, the address's page is unmapped.
	if (!(p3Table[p3Index] & PagePresent)) {
		return ResultSerialOutputUnavailabe;
	}

	u16 p2Index = VirtualAddressPage2Index(address);
	PageTableEntry* p2Table = PhysicalAddressAsPointer(p3Table[p3Index] & ~(0xfff));

	// If there is no Level 1 table at the expected level 2's index, the address's page is unmapped.
	if (!(p2Table[p2Index] & PagePresent)) {
		return ResultSerialOutputUnavailabe;
	}

	u16 p1Index = VirtualAddressPage1Index(address);
	PageTableEntry* p1Table = PhysicalAddressAsPointer(p2Table[p2Index] & ~(0xfff));

	// If the entry at the expected level 1's index is empty, the address's page is unmapped.
	if (!(p1Table[p1Index] & PagePresent)) {
		return ResultSerialOutputUnavailabe;
	}

	*physicalAddress = p1Table[p1Index] & ~(0xfff);
	*physicalAddress |= address & PAGE_OFFSET_MASK;

	return ResultOk;
}
