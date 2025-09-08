#include "Memory/Page.h"

#include "Logger.h"
#include "Memory/BitmapFrameAllocator.h"
#include "Memory/Frame.h"
#include "Memory/PageTable.h"
#include "Memory/VirtualAddress.h"

Result Page4KiBMap(PageTableEntry* p4Table, Page4KiB page, Frame4KiB frame, PageTableEntryFlags flags)
{
	u16 p4Index = VirtualAddressPage4Index(page);
	PageTableEntryFlags userAccessible = flags & PageUserAccessible;

	// If there is no Level 3 table at the expected level 4's index, we need to create it
	if (!(p4Table[p4Index] & PagePresent)) {
		Frame4KiB newP3 = AllocateFrame(&g_frameAllocator);

		// The newly created Level 3 table should be empty
		MemoryFill(PhysicalAddressAsPointer(newP3), 0, FRAME_4KIB_SIZE_BYTES);

		p4Table[p4Index] = newP3 | PagePresent | PageWriteable;
	}

	// If the entry pointing to the Level 3 table is not user accessible but it should be, we make it user accessible
	p4Table[p4Index] |= userAccessible;

	u16 p3Index = VirtualAddressPage3Index(page);
	PageTableEntry* p3Table = PhysicalAddressAsPointer(p4Table[p4Index] & ~(0xfff));

	// If there is no Level 2 table at the expected level 3's index, we need to create it
	if (!(p3Table[p3Index] & PagePresent)) {
		Frame4KiB newP2 = AllocateFrame(&g_frameAllocator);

		// The newly created Level 2 table should be empty
		MemoryFill(PhysicalAddressAsPointer(newP2), 0, FRAME_4KIB_SIZE_BYTES);

		p3Table[p3Index] = newP2 | PagePresent | PageWriteable;
	}

	// If the entry pointing to the Level 2 table is not user accessible but it should be, we make it user accessible
	p3Table[p3Index] |= userAccessible;

	u16 p2Index = VirtualAddressPage2Index(page);
	PageTableEntry* p2Table = PhysicalAddressAsPointer(p3Table[p3Index] & ~(0xfff));

	// If there is no Level 1 table at the expected level 2's index, we need to create it
	if (!(p2Table[p2Index] & PagePresent)) {
		Frame4KiB newP1 = AllocateFrame(&g_frameAllocator);

		// The newly created Level 1 table should be empty
		MemoryFill(PhysicalAddressAsPointer(newP1), 0, FRAME_4KIB_SIZE_BYTES);

		p2Table[p2Index] = newP1 | PagePresent | PageWriteable;
	}

	// If the entry pointing to the Level 1 table is not user accessible but it should be, we make it user accessible
	p2Table[p2Index] |= userAccessible;

	u16 p1Index = VirtualAddressPage1Index(page);
	PageTableEntry* p1Table = PhysicalAddressAsPointer(p2Table[p2Index] & ~(0xfff));

	// If we are trying to map an existing page, something went really wrong...
	if (p1Table[p1Index] & PagePresent) {
		LogLine(SK_LOG_WARN "An attempt was made to map an existing page table entry");
		return ResultPageAlreadyMapped;
	}

	p1Table[p1Index] = frame | flags | PagePresent;

	return ResultOk;
}

Result Page4KiBUnmap(const PageTableEntry* p4Table, Page4KiB page)
{
	u16 p4Index = VirtualAddressPage4Index(page);

	// If there is no Level 3 table at the expected level 4's index, this virtual address is not mapped.
	if (!(p4Table[p4Index] & PagePresent)) {
		LogLine(SK_LOG_WARN "An attempt was made to unmap an already unmapped page");
		return ResultPageAlreadyUnmapped;
	}

	u16 p3Index = VirtualAddressPage3Index(page);
	PageTableEntry* p3Table = PhysicalAddressAsPointer(p4Table[p4Index] & ~(0xfff));

	// If there is no Level 2 table at the expected level 3's index, this virtual address is not mapped.
	if (!(p3Table[p3Index] & PagePresent)) {
		LogLine(SK_LOG_WARN "An attempt was made to unmap an already unmapped page");
		return ResultPageAlreadyUnmapped;
	}

	u16 p2Index = VirtualAddressPage2Index(page);
	PageTableEntry* p2Table = PhysicalAddressAsPointer(p3Table[p3Index] & ~(0xfff));

	// If there is no Level 1 table at the expected level 2's index, this virtual address is not mapped.
	if (!(p2Table[p2Index] & PagePresent)) {
		LogLine(SK_LOG_WARN "An attempt was made to unmap an already unmapped page");
		return ResultPageAlreadyUnmapped;
	}

	u16 p1Index = VirtualAddressPage1Index(page);
	PageTableEntry* p1Table = PhysicalAddressAsPointer(p2Table[p2Index] & ~(0xfff));

	// If there is no entry at the expected level 1's index, this virtual address is not mapped.
	if (!(p1Table[p1Index] & PagePresent)) {
		LogLine(SK_LOG_WARN "An attempt was made to unmap an already unmapped page");
		return ResultPageAlreadyUnmapped;
	}

	p1Table[p1Index] = 0;

	return ResultOk;
}

Result Page4KiBRemap(PageTableEntry* p4Table, Page4KiB page, Frame4KiB frame, PageTableEntryFlags flags)
{
	u16 p4Index = VirtualAddressPage4Index(page);

	// If there is no Level 3 table at the expected level 4's index, this virtual address is not mapped.
	if (!(p4Table[p4Index] & PagePresent)) {
		LogLine(SK_LOG_WARN "An attempt was made to remap an unmapped page");
		return ResultPageAlreadyUnmapped;
	}

	p4Table[p4Index] |= flags;

	u16 p3Index = VirtualAddressPage3Index(page);
	PageTableEntry* p3Table = PhysicalAddressAsPointer(p4Table[p4Index] & ~(0xfff));

	// If there is no Level 2 table at the expected level 3's index, this virtual address is not mapped.
	if (!(p3Table[p3Index] & PagePresent)) {
		LogLine(SK_LOG_WARN "An attempt was made to remap an unmapped page");
		return ResultPageAlreadyUnmapped;
	}

	p3Table[p3Index] |= flags;

	u16 p2Index = VirtualAddressPage2Index(page);
	PageTableEntry* p2Table = PhysicalAddressAsPointer(p3Table[p3Index] & ~(0xfff));

	// If there is no Level 1 table at the expected level 2's index, this virtual address is not mapped.
	if (!(p2Table[p2Index] & PagePresent)) {
		LogLine(SK_LOG_WARN "An attempt was made to remap an unmapped page");
		return ResultPageAlreadyUnmapped;
	}

	p2Table[p2Index] |= flags;

	u16 p1Index = VirtualAddressPage1Index(page);
	PageTableEntry* p1Table = PhysicalAddressAsPointer(p2Table[p2Index] & ~(0xfff));

	// If there is no entry at the expected level 1's index, this virtual address is not mapped.
	if (!(p1Table[p1Index] & PagePresent)) {
		LogLine(SK_LOG_WARN "An attempt was made to remap an unmapped page");
		return ResultPageAlreadyUnmapped;
	}

	p1Table[p1Index] = frame | flags | PagePresent;

	return ResultOk;
}
