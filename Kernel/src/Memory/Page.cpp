#include "Memory/Page.hpp"

#include "Logger.hpp"
#include "Memory/BitmapFrameAllocator.hpp"
#include "Memory/Frame.hpp"
#include "Memory/PageTable.hpp"

namespace SaturnKernel {

Page<Size4KiB>::Page(u64 address)
	: Address(address & ~0xfff)
{
}

Page<Size4KiB>::Page(VirtualAddress address)
	: Address(address.Value & ~0xfff)
{
}

auto Page<Size4KiB>::operator++(int) -> Page
{
	Page temp = *this;
	Address += SIZE_BYTES;
	return temp;
}

auto Page<Size4KiB>::operator--(int) -> Page
{
	Page temp = *this;
	Address -= SIZE_BYTES;
	return temp;
}

auto Page<Size4KiB>::operator<=(const Page& other) const -> bool { return Address <= other.Address; }

auto Page<Size4KiB>::operator>=(const Page& other) const -> bool { return Address >= other.Address; }

auto Page<Size4KiB>::operator<(const Page& other) const -> bool { return Address < other.Address; }

auto Page<Size4KiB>::operator>(const Page& other) const -> bool { return Address > other.Address; }

auto Page<Size4KiB>::MapTo(const Frame<Size4KiB>& frame, PageTableEntryFlags flags) const -> Result<void>
{
	u16 p4Index = Address.Page4Index();
	auto* p4Table = PageTable4Address().AsPointer<PageTableEntry>();

	// If there is no Level 3 table at the expected level 4's index, we need to create it.
	if ((p4Table[p4Index].Flags() & PageTableEntryFlags::Present) != PageTableEntryFlags::Present) {
		auto newP3 = g_frameAllocator.AllocateFrame();
		if (newP3.IsError()) {
			SK_LOG_ERROR("An unexpected error occured while trying to allocate a memory frame");
			return Result<void>::MakeErr(newP3.Error);
		}

		// The newly created Level 3 table should be empty
		MemoryFill(newP3.Value.UsableAddress(), 0, 4096);

		p4Table[p4Index].Flags(PageTableEntryFlags::Present | PageTableEntryFlags::Writeable);
		p4Table[p4Index].PhysicalFrameAddress(newP3.Value.Address);
	}

	u16 p3Index = Address.Page3Index();
	auto* p3Table = p4Table[p4Index].PhysicalFrameAddress().AsPointer<PageTableEntry>();

	// If there is no Level 2 table at the expected level 3's index, we need to create it.
	if ((p3Table[p3Index].Flags() & PageTableEntryFlags::Present) != PageTableEntryFlags::Present) {
		auto newP2 = g_frameAllocator.AllocateFrame();
		if (newP2.IsError()) {
			SK_LOG_ERROR("An unexpected error occured while trying to allocate a memory frame");
			return Result<void>::MakeErr(newP2.Error);
		}

		// The newly created Level 2 table should be empty
		MemoryFill(newP2.Value.UsableAddress(), 0, 4096);

		p3Table[p3Index].Flags(PageTableEntryFlags::Present | PageTableEntryFlags::Writeable);
		p3Table[p3Index].PhysicalFrameAddress(newP2.Value.Address);
	}

	u16 p2Index = Address.Page2Index();
	auto* p2Table = p3Table[p3Index].PhysicalFrameAddress().AsPointer<PageTableEntry>();

	// If there is no Level 1 table at the expected level 2's index, we need to create it.
	if ((p2Table[p2Index].Flags() & PageTableEntryFlags::Present) != PageTableEntryFlags::Present) {
		auto newP1 = g_frameAllocator.AllocateFrame();
		if (newP1.IsError()) {
			SK_LOG_ERROR("An unexpected error occured while trying to allocate a memory frame");
			return Result<void>::MakeErr(newP1.Error);
		}

		// The newly created Level 1 table should be empty
		MemoryFill(newP1.Value.UsableAddress(), 0, 4096);

		p2Table[p2Index].Flags(PageTableEntryFlags::Present | PageTableEntryFlags::Writeable);
		p2Table[p2Index].PhysicalFrameAddress(newP1.Value.Address);
	}

	u16 p1Index = Address.Page1Index();
	auto* p1Table = p2Table[p2Index].PhysicalFrameAddress().AsPointer<PageTableEntry>();

	// If we are trying to map an existing page, something went really wrong...
	if ((p1Table[p1Index].Flags() & PageTableEntryFlags::Present) == PageTableEntryFlags::Present) {
		SK_LOG_WARN("An attempt was made to map an existing page table entry");
		return Result<void>::MakeErr(ErrorCode::PageAlreadyMapped);
	}

	p1Table[p1Index].Flags(flags | PageTableEntryFlags::Present);
	p1Table[p1Index].PhysicalFrameAddress(frame.Address);

	return Result<void>::MakeOk();
}

auto Page<Size4KiB>::Unmap() const -> Result<void>
{
	u16 p4Index = Address.Page4Index();
	auto* p4Table = PageTable4Address().AsPointer<PageTableEntry>();

	// If there is no Level 3 table at the expected level 4's index, this virtual address is not mapped.
	if ((p4Table[p4Index].Flags() & PageTableEntryFlags::Present) != PageTableEntryFlags::Present) {
		SK_LOG_WARN("An attempt was made to unmap an already unmapped page");
		return Result<void>::MakeErr(ErrorCode::PageAlreadyUnmapped);
	}

	u16 p3Index = Address.Page3Index();
	auto* p3Table = p4Table[p4Index].PhysicalFrameAddress().AsPointer<PageTableEntry>();

	// If there is no Level 2 table at the expected level 3's index, this virtual address is not mapped.
	if ((p3Table[p3Index].Flags() & PageTableEntryFlags::Present) != PageTableEntryFlags::Present) {
		SK_LOG_WARN("An attempt was made to unmap an already unmapped page");
		return Result<void>::MakeErr(ErrorCode::PageAlreadyUnmapped);
	}

	u16 p2Index = Address.Page2Index();
	auto* p2Table = p3Table[p3Index].PhysicalFrameAddress().AsPointer<PageTableEntry>();

	// If there is no Level 1 table at the expected level 2's index, this virtual address is not mapped.
	if ((p2Table[p2Index].Flags() & PageTableEntryFlags::Present) != PageTableEntryFlags::Present) {
		SK_LOG_WARN("An attempt was made to unmap an already unmapped page");
		return Result<void>::MakeErr(ErrorCode::PageAlreadyUnmapped);
	}

	u16 p1Index = Address.Page1Index();
	auto* p1Table = p2Table[p2Index].PhysicalFrameAddress().AsPointer<PageTableEntry>();

	// If there is no entry at the expected level 1's index, this virtual address is not mapped.
	if ((p1Table[p1Index].Flags() & PageTableEntryFlags::Present) != PageTableEntryFlags::Present) {
		SK_LOG_WARN("An attempt was made to unmap an already unmapped page");
		return Result<void>::MakeErr(ErrorCode::PageAlreadyUnmapped);
	}

	p1Table[p1Index].Clear();

	return Result<void>::MakeOk();
}

}
