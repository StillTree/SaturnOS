#include "Memory/VirtualAddress.hpp"

namespace SaturnKernel {

VirtualAddress::VirtualAddress(U64 address)
	: Address(address)
{
}

[[nodiscard]] auto VirtualAddress::PageOffset() const -> U16 { return Address & PAGE_OFFSET_MASK; }

[[nodiscard]] auto VirtualAddress::Page1Index() const -> U16 { return (Address >> 12) & INDEX_MASK; }

[[nodiscard]] auto VirtualAddress::Page2Index() const -> U16 { return (Address >> 21) & INDEX_MASK; }

[[nodiscard]] auto VirtualAddress::Page3Index() const -> U16 { return (Address >> 30) & INDEX_MASK; }

[[nodiscard]] auto VirtualAddress::Page4Index() const -> U16 { return (Address >> 39) & INDEX_MASK; }

auto VirtualAddress::operator+=(U64 other) -> VirtualAddress&
{
	Address += other;
	return *this;
}

auto VirtualAddress::operator-=(U64 other) -> VirtualAddress&
{
	Address -= other;
	return *this;
}

}
