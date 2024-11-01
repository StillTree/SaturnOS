#pragma once

#include "Core.hpp"

#include "Memory.hpp"
#include "Memory/Page.hpp"
#include "Result.hpp"

namespace SaturnKernel {

template <typename T> struct Frame;

/// Represents a 4 KiB physical memory frame.
template <> struct Frame<Size4KiB> {
	/// Aligns down the given address to the lower `SIZE_BYTES` byte if it already isn't.
	explicit Frame<Size4KiB>(U64 address);

	/// Increments the frame, to the next one.
	auto operator++(int) -> Frame;
	/// Decrements the frame, to the previous one.
	auto operator--(int) -> Frame;
	auto operator<(const Frame& other) const -> bool;
	auto operator>(const Frame& other) const -> bool;
	auto operator+(U64 other) const -> Frame;
	auto operator-(U64 other) const -> Frame;

	/// Maps this physical memory frame to the given virtual memory page, using the global frame allocator if needed.
	auto MapTo(const Page<Size4KiB>& page) -> Result<void>;

	/// The frame's first address.
	U64 Address;

	/// The frame's size in bytes.
	static constexpr U64 SIZE_BYTES = 4096;
};

}
