#pragma once

#include "Core.hpp"

namespace SaturnKernel {

struct PhysicalAddress {
	explicit PhysicalAddress(U64 address);

	template <typename T> [[nodiscard]] auto AsPointer() const -> T*
	{
		return reinterpret_cast<T*>(Value + g_bootInfo.PhysicalMemoryOffset);
	}

	auto operator+=(U64 other) -> PhysicalAddress&;
	auto operator+(U64 other) const -> PhysicalAddress;
	auto operator-=(U64 other) -> PhysicalAddress&;
	auto operator-(U64 other) const -> PhysicalAddress;
	auto operator-(PhysicalAddress other) const -> U64;
	auto operator/(U64 other) const -> U64;
	auto operator<(const PhysicalAddress& other) const -> bool;
	auto operator>(const PhysicalAddress& other) const -> bool;
	auto operator<=(const PhysicalAddress& other) const -> bool;
	auto operator>=(const PhysicalAddress& other) const -> bool;

	U64 Value;
};

}
