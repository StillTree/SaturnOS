#pragma once

#include "Core.hpp"
#include "Memory.hpp"
#include "Memory/PhysicalAddress.hpp"

namespace SaturnKernel {

template <typename T> struct Frame;

/// Represents a 4 KiB physical memory frame.
template <> struct Frame<Size4KiB> {
	/// Aligns down the given address to the lower `SIZE_BYTES` byte if it already isn't.
	explicit Frame(u64 address);
	/// Aligns down the given address to the lower `SIZE_BYTES` byte if it already isn't.
	explicit Frame(PhysicalAddress address);

	auto operator++(int) -> Frame;
	auto operator--(int) -> Frame;
	auto operator++() -> Frame;
	auto operator--() -> Frame;
	auto operator<(const Frame& other) const -> bool;
	auto operator>(const Frame& other) const -> bool;
	auto operator<=(const Frame& other) const -> bool;
	auto operator>=(const Frame& other) const -> bool;
	auto operator+(u64 other) const -> Frame;
	auto operator-(u64 other) const -> Frame;

	/// Returns a `void*` with a usable address, adjusted for the physical memory mapping performed by the bootloader.
	[[nodiscard]] auto UsableAddress() const -> void*;

	/// The frame's first address.
	PhysicalAddress Address;

	/// The frame's size in bytes.
	static constexpr u64 SIZE_BYTES = 4096;
};

}
