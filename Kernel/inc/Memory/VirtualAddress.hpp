#pragma once

#include "Core.hpp"

namespace SaturnKernel {

struct VirtualAddress {
	explicit VirtualAddress(u64 address);

	[[nodiscard]] auto PageOffset() const -> u16;
	[[nodiscard]] auto Page1Index() const -> u16;
	[[nodiscard]] auto Page2Index() const -> u16;
	[[nodiscard]] auto Page3Index() const -> u16;
	[[nodiscard]] auto Page4Index() const -> u16;

	template <typename T> [[nodiscard]] auto AsPointer() const -> T* { return reinterpret_cast<T*>(Value); }

	auto operator+=(u64 other) -> VirtualAddress&;
	auto operator+(u64 other) const -> VirtualAddress;
	auto operator-=(u64 other) -> VirtualAddress&;
	auto operator-(u64 other) const -> VirtualAddress;
	auto operator<=(const VirtualAddress& other) const -> bool;
	auto operator>=(const VirtualAddress& other) const -> bool;
	auto operator<(const VirtualAddress& other) const -> bool;
	auto operator>(const VirtualAddress& other) const -> bool;

	u64 Value;
};

inline constexpr u64 INDEX_MASK = ((1ULL << 9) - 1);
inline constexpr u64 PAGE_OFFSET_MASK = 0xfff;

}
