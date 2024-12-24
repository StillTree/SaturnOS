#pragma once

#include "Core.hpp"
#include "Memory/PhysicalAddress.hpp"

namespace SaturnKernel {

enum class PageTableEntryFlags : U64 {
	Present = 1ULL,
	Writeable = 1ULL << 1,
	UserAccessible = 1ULL << 2,
	WriteThroughCaching = 1ULL << 3,
	NoCache = 1ULL << 4,
	Accessed = 1ULL << 5,
	Dirty = 1ULL << 6,
	HugePage = 1ULL << 7,
	Global = 1ULL << 8,
	NoExecute = 1ULL << 63,
};

inline auto operator|(PageTableEntryFlags a, PageTableEntryFlags b) -> PageTableEntryFlags
{
	return static_cast<PageTableEntryFlags>(static_cast<U64>(a) | static_cast<U64>(b));
}

inline auto operator&(PageTableEntryFlags a, PageTableEntryFlags b) -> PageTableEntryFlags
{
	return static_cast<PageTableEntryFlags>(static_cast<U64>(a) & static_cast<U64>(b));
}

struct PageTableEntry {
	explicit PageTableEntry(U64 entry);

	[[nodiscard]] auto Flags() const -> PageTableEntryFlags;
	[[nodiscard]] auto PhysicalFrameAddress() const -> PhysicalAddress;

	auto Flags(PageTableEntryFlags flags) -> void;
	auto PhysicalFrameAddress(PhysicalAddress address) -> void;

	U64 Entry;

	static constexpr U64 FRAME_ADDRESS_MASK = (1ULL << 40) - 1;
	static constexpr U64 FLAGS_MASK = 0xfff;
	static constexpr U64 PAGE_TABLE_ENTRIES = 512;
};

/// Returns the PML4 table's physical memory address, read from the CR3 register.
inline auto PageTable4Address() -> PhysicalAddress
{
	U64 pml4Address = -1;
	__asm__ volatile("mov %%cr3, %0" : "=r"(pml4Address));

	return PhysicalAddress(pml4Address);
}

}
