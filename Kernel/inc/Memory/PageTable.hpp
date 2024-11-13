#pragma once

#include "Core.hpp"

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
	[[nodiscard]] auto PhysicalFrameAddress() const -> U64;

	auto Flags(PageTableEntryFlags flags) -> void;
	auto PhysicalFrameAddress(U64 address) -> void;

	U64 Entry;
};

inline auto PageTable4Address() -> U64
{
	U64 pml4Address = -1;
	__asm__ volatile("mov %%cr3, %0" : "=r"(pml4Address));

	return pml4Address;
}

constexpr inline U64 FRAME_ADDRESS_MASK = (1ULL << 40) - 1;
constexpr inline U64 FLAGS_MASK = 0xfff;
constexpr inline U64 PAGE_TABLE_ENTRIES = 512;

}
