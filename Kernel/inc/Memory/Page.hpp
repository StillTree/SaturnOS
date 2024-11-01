#pragma once

#include "Core.hpp"

#include "Memory.hpp"

namespace SaturnKernel {

template <typename T> struct Page;

/// Represents a 4 KiB virtual memory page.
template <> struct Page<Size4KiB> {
	/// Aligns down the given address to the lower `SIZE_BYTES` byte if it already isn't.
	explicit Page(U64 address);

	/// Increments the page, to the next one.
	auto operator++(int) -> Page;
	/// Decrements the page, to the previous one.
	auto operator--(int) -> Page;

	/// The page's first address.
	U64 Address;

	/// The page's size in bytes.
	static constexpr U64 SIZE_BYTES = 4096;
};

}
