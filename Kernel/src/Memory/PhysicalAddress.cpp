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

auto PhysicalAddress::operator+(U64 other) const -> PhysicalAddress { return PhysicalAddress(Value + other); }

auto PhysicalAddress::operator-=(U64 other) -> PhysicalAddress&
{
	Value -= other;
	return *this;
}

auto PhysicalAddress::operator-(U64 other) const -> PhysicalAddress { return PhysicalAddress(Value - other); }

auto PhysicalAddress::operator-(PhysicalAddress other) const -> U64 { return Value - other.Value; }

auto PhysicalAddress::operator/(U64 other) const -> U64 { return Value / other; }

auto PhysicalAddress::operator<=(const PhysicalAddress& other) const -> bool { return Value <= other.Value; }

auto PhysicalAddress::operator>=(const PhysicalAddress& other) const -> bool { return Value >= other.Value; }

auto PhysicalAddress::operator<(const PhysicalAddress& other) const -> bool { return Value < other.Value; }

auto PhysicalAddress::operator>(const PhysicalAddress& other) const -> bool { return Value > other.Value; }

}
