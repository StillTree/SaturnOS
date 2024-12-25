#pragma once

#include "Core.hpp"

namespace SaturnKernel {

struct VirtualAddress {
	explicit VirtualAddress(U64 address);

	[[nodiscard]] auto PageOffset() const -> U16;
	[[nodiscard]] auto Page1Index() const -> U16;
	[[nodiscard]] auto Page2Index() const -> U16;
	[[nodiscard]] auto Page3Index() const -> U16;
	[[nodiscard]] auto Page4Index() const -> U16;

	template <typename T> [[nodiscard]] auto AsPointer() const -> T* { return reinterpret_cast<T*>(Value); }

	auto operator+=(U64 other) -> VirtualAddress&;
	auto operator+(U64 other) const -> VirtualAddress;
	auto operator-=(U64 other) -> VirtualAddress&;
	auto operator-(U64 other) const -> VirtualAddress;
	auto operator<=(const VirtualAddress& other) const -> bool;
	auto operator>=(const VirtualAddress& other) const -> bool;

	U64 Value;
};

inline constexpr U64 INDEX_MASK = ((1ULL << 9) - 1);
inline constexpr U64 PAGE_OFFSET_MASK = 0xfff;

}
