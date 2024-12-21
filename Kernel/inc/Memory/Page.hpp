#pragma once

#include "Core.hpp"

#include "Memory.hpp"
#include "Memory/VirtualAddress.hpp"

namespace SaturnKernel {

template <typename T> struct Page;

/// Represents a 4 KiB virtual memory page.
template <> struct Page<Size4KiB> {
	/// Aligns down the given address to the lower `SIZE_BYTES` byte if it already isn't.
	explicit Page<Size4KiB>(U64 address);
	/// Aligns down the given address to the lower `SIZE_BYTES` byte if it already isn't.
	explicit Page<Size4KiB>(VirtualAddress address);

	auto operator++(int) -> Page;
	auto operator--(int) -> Page;
	auto operator<=(const Page& other) const -> bool;
	auto operator>=(const Page& other) const -> bool;

	/// The page's first address.
	VirtualAddress Address;

	/// The page's size in bytes.
	static constexpr U64 SIZE_BYTES = 4096;
};

}
