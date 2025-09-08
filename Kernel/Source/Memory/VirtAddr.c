#include "Memory/VirtAddr.h"

Result VirtAddrToPhys(const PageTableEntry* p4Table, VirtAddr address, PhysAddr* physAddr)
{
	u16 p4Index = VirtAddrPage4Index(address);

	// If there is no Level 3 table at the expected level 4's index, the address's page is unmapped.
	if (!(p4Table[p4Index] & PagePresent)) {
		return ResultSerialOutputUnavailable;
	}

	u16 p3Index = VirtAddrPage3Index(address);
	PageTableEntry* p3Table = PhysAddrAsPointer(p4Table[p4Index] & ~(0xfff));

	// If there is no Level 2 table at the expected level 3's index, the address's page is unmapped.
	if (!(p3Table[p3Index] & PagePresent)) {
		return ResultSerialOutputUnavailable;
	}

	u16 p2Index = VirtAddrPage2Index(address);
	PageTableEntry* p2Table = PhysAddrAsPointer(p3Table[p3Index] & ~(0xfff));

	// If there is no Level 1 table at the expected level 2's index, the address's page is unmapped.
	if (!(p2Table[p2Index] & PagePresent)) {
		return ResultSerialOutputUnavailable;
	}

	u16 p1Index = VirtAddrPage1Index(address);
	PageTableEntry* p1Table = PhysAddrAsPointer(p2Table[p2Index] & ~(0xfff));

	// If the entry at the expected level 1's index is empty, the address's page is unmapped.
	if (!(p1Table[p1Index] & PagePresent)) {
		return ResultSerialOutputUnavailable;
	}

	*physAddr = p1Table[p1Index] & ~(0xfff | PageNoExecute);
	*physAddr |= address & PAGE_OFFSET_MASK;

	return ResultOk;
}
