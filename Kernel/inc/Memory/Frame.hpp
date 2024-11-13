#pragma once

#include "Core.hpp"

#include "Memory.hpp"
#include "Memory/Page.hpp"
#include "Memory/PageTable.hpp"
#include "Result.hpp"

namespace SaturnKernel {

template <typename T> struct Frame;

/// Represents a 4 KiB physical memory frame.
template <> struct Frame<Size4KiB> {
	/// Aligns down the given address to the lower `SIZE_BYTES` byte if it already isn't.
	explicit Frame<Size4KiB>(U64 address);

	auto operator++(int) -> Frame;
	auto operator--(int) -> Frame;
	auto operator++() -> Frame;
	auto operator--() -> Frame;
	auto operator<(const Frame& other) const -> bool;
	auto operator>(const Frame& other) const -> bool;
	auto operator<=(const Frame& other) const -> bool;
	auto operator>=(const Frame& other) const -> bool;
	auto operator+(U64 other) const -> Frame;
	auto operator-(U64 other) const -> Frame;

	/// Returns a `void*` with a usable address, adjusted for the physical memory mapping performed by the bootloader.
	[[nodiscard]] auto UsableAddress() const -> void*;

	/// Maps this physical memory frame to the given virtual memory page, using the global frame allocator if needed.
	/// Does not flush the TLB.
	auto MapTo(const Page<Size4KiB>& page, PageTableEntryFlags flags) const -> Result<void>;

	/// The frame's first address.
	U64 Address;

	/// The frame's size in bytes.
	static constexpr U64 SIZE_BYTES = 4096;
};

}
