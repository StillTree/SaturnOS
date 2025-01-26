#pragma once

#include "Core.hpp"

namespace SaturnKernel {

struct PhysicalAddress {
	explicit PhysicalAddress(u64 address);

	template <typename T> [[nodiscard]] auto AsPointer() const -> T*
	{
		return reinterpret_cast<T*>(Value + g_bootInfo.PhysicalMemoryOffset);
	}

	template <typename T> [[nodiscard]] auto AsRawPointer() const -> T*
	{
		return reinterpret_cast<T*>(Value);
	}

	auto operator+=(u64 other) -> PhysicalAddress&;
	auto operator+(u64 other) const -> PhysicalAddress;
	auto operator-=(u64 other) -> PhysicalAddress&;
	auto operator-(u64 other) const -> PhysicalAddress;
	auto operator-(PhysicalAddress other) const -> u64;
	auto operator/(u64 other) const -> u64;
	auto operator<(const PhysicalAddress& other) const -> bool;
	auto operator>(const PhysicalAddress& other) const -> bool;
	auto operator<=(const PhysicalAddress& other) const -> bool;
	auto operator>=(const PhysicalAddress& other) const -> bool;

	u64 Value;
};

}
