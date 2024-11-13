#include "Memory/Frame.hpp"

#include "Core.hpp"
#include "Logger.hpp"
#include "Memory/BitmapFrameAllocator.hpp"

namespace SaturnKernel {

Frame<Size4KiB>::Frame(U64 address)
	: Address(address & ~0xfff)
{
}

auto Frame<Size4KiB>::operator++(int) -> Frame
{
	Frame temp = *this;
	Address += SIZE_BYTES;
	return temp;
}

auto Frame<Size4KiB>::operator--(int) -> Frame
{
	Frame temp = *this;
	Address -= SIZE_BYTES;
	return temp;
}

auto Frame<Size4KiB>::operator++() -> Frame
{
	Address += SIZE_BYTES;
	return *this;
}
auto Frame<Size4KiB>::operator--() -> Frame
{
	Address -= SIZE_BYTES;
	return *this;
}

auto Frame<Size4KiB>::operator<(const Frame& other) const -> bool { return Address < other.Address; }

auto Frame<Size4KiB>::operator>(const Frame& other) const -> bool { return Address > other.Address; }

auto Frame<Size4KiB>::operator<=(const Frame& other) const -> bool { return Address <= other.Address; }

auto Frame<Size4KiB>::operator>=(const Frame& other) const -> bool { return Address >= other.Address; }

auto Frame<Size4KiB>::operator+(U64 other) const -> Frame { return Frame(Address + (other * 4096)); }

auto Frame<Size4KiB>::operator-(U64 other) const -> Frame { return Frame(Address - (other * 4096)); }

[[nodiscard]] auto Frame<Size4KiB>::UsableAddress() const -> void*
{
	return reinterpret_cast<void*>(Address + g_bootInfo.PhysicalMemoryOffset);
}

auto Frame<Size4KiB>::MapTo(const Page<Size4KiB>& page, PageTableEntryFlags flags) const -> Result<void>
{
	U16 p4Index = page.Address.Page4Index();
	auto* p4Table = reinterpret_cast<PageTableEntry*>(PageTable4Address() + g_bootInfo.PhysicalMemoryOffset);

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

	U16 p3Index = page.Address.Page3Index();
	auto* p3Table = reinterpret_cast<PageTableEntry*>(p4Table[p4Index].PhysicalFrameAddress() + g_bootInfo.PhysicalMemoryOffset);

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

	U16 p2Index = page.Address.Page2Index();
	auto* p2Table = reinterpret_cast<PageTableEntry*>(p3Table[p3Index].PhysicalFrameAddress() + g_bootInfo.PhysicalMemoryOffset);

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

	U16 p1Index = page.Address.Page1Index();
	auto* p1Table = reinterpret_cast<PageTableEntry*>(p2Table[p2Index].PhysicalFrameAddress() + g_bootInfo.PhysicalMemoryOffset);

	// If we are trying to map an existing page, something went really wrong...
	if ((p1Table[p1Index].Flags() & PageTableEntryFlags::Present) == PageTableEntryFlags::Present) {
		SK_LOG_WARN("An attempt was made to map an existing page table entry");
		return Result<void>::MakeErr(ErrorCode::FrameAlreadyMapped);
	}

	p1Table[p1Index].Flags(flags | PageTableEntryFlags::Present);
	p1Table[p1Index].PhysicalFrameAddress(Address);

	return Result<void>::MakeOk();
}

}
