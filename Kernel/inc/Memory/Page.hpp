#pragma once

#include "Core.hpp"

#include "Memory.hpp"
#include "Memory/PageTable.hpp"
#include "Memory/Frame.hpp"
#include "Memory/VirtualAddress.hpp"
#include "Result.hpp"

namespace SaturnKernel {

template <typename T> struct Page;

/// Represents a 4 KiB virtual memory page.
template <> struct Page<Size4KiB> {
	/// Aligns down the given address to the lower `SIZE_BYTES` byte if it already isn't.
	explicit Page(U64 address);
	/// Aligns down the given address to the lower `SIZE_BYTES` byte if it already isn't.
	explicit Page(VirtualAddress address);

	auto operator++(int) -> Page;
	auto operator--(int) -> Page;
	auto operator<=(const Page& other) const -> bool;
	auto operator>=(const Page& other) const -> bool;

	/// Maps this virtual memory page to the given physical memory frame, using the global frame allocator if needed.
	/// Does not flush the TLB.
	auto MapTo(const Frame<Size4KiB>& frame, PageTableEntryFlags flags) const -> Result<void>;
	/// Clears the page table entry associated with this page.
	/// Does not flush the TLB.
	auto Unmap() const -> Result<void>;

	/// The page's first address.
	VirtualAddress Address;

	/// The page's size in bytes.
	static constexpr U64 SIZE_BYTES = 4096;
};

}
