#include "Memory/PhysicalAddress.hpp"
#include "Core.hpp"

namespace SaturnKernel {

PhysicalAddress::PhysicalAddress(U64 address)
	: Value(address)
{
}

auto PhysicalAddress::operator+=(U64 other) -> PhysicalAddress&
{
	Value += other;
	return *this;
}

auto PhysicalAddress::operator-=(U64 other) -> PhysicalAddress&
{
	Value -= other;
	return *this;
}

auto PhysicalAddress::operator<=(const PhysicalAddress& other) const -> bool { return Value <= other.Value; }

auto PhysicalAddress::operator>=(const PhysicalAddress& other) const -> bool { return Value >= other.Value; }

auto PhysicalAddress::operator<(const PhysicalAddress& other) const -> bool { return Value < other.Value; }

auto PhysicalAddress::operator>(const PhysicalAddress& other) const -> bool { return Value > other.Value; }

}
