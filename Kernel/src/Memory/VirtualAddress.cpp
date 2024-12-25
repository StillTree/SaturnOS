#include "Memory/VirtualAddress.hpp"

namespace SaturnKernel {

VirtualAddress::VirtualAddress(U64 address)
	: Value(address)
{
}

[[nodiscard]] auto VirtualAddress::PageOffset() const -> U16 { return Value & PAGE_OFFSET_MASK; }

[[nodiscard]] auto VirtualAddress::Page1Index() const -> U16 { return (Value >> 12) & INDEX_MASK; }

[[nodiscard]] auto VirtualAddress::Page2Index() const -> U16 { return (Value >> 21) & INDEX_MASK; }

[[nodiscard]] auto VirtualAddress::Page3Index() const -> U16 { return (Value >> 30) & INDEX_MASK; }

[[nodiscard]] auto VirtualAddress::Page4Index() const -> U16 { return (Value >> 39) & INDEX_MASK; }

auto VirtualAddress::operator+=(U64 other) -> VirtualAddress&
{
	Value += other;
	return *this;
}

auto VirtualAddress::operator+(U64 other) const -> VirtualAddress 
{
	return VirtualAddress(Value + other);
}

auto VirtualAddress::operator-=(U64 other) -> VirtualAddress&
{
	Value -= other;
	return *this;
}

auto VirtualAddress::operator-(U64 other) const -> VirtualAddress 
{
	return VirtualAddress(Value - other);
}

auto VirtualAddress::operator<=(const VirtualAddress& other) const -> bool { return Value <= other.Value; }

auto VirtualAddress::operator>=(const VirtualAddress& other) const -> bool { return Value >= other.Value; }

}
