#include "Memory/PhysicalAddress.hpp"

#include "Core.hpp"

namespace SaturnKernel {

PhysicalAddress::PhysicalAddress(u64 address)
	: Value(address)
{
}

auto PhysicalAddress::operator+=(u64 other) -> PhysicalAddress&
{
	Value += other;
	return *this;
}

auto PhysicalAddress::operator+(u64 other) const -> PhysicalAddress { return PhysicalAddress(Value + other); }

auto PhysicalAddress::operator-=(u64 other) -> PhysicalAddress&
{
	Value -= other;
	return *this;
}

auto PhysicalAddress::operator-(u64 other) const -> PhysicalAddress { return PhysicalAddress(Value - other); }

auto PhysicalAddress::operator-(PhysicalAddress other) const -> u64 { return Value - other.Value; }

auto PhysicalAddress::operator/(u64 other) const -> u64 { return Value / other; }

auto PhysicalAddress::operator<=(const PhysicalAddress& other) const -> bool { return Value <= other.Value; }

auto PhysicalAddress::operator>=(const PhysicalAddress& other) const -> bool { return Value >= other.Value; }

auto PhysicalAddress::operator<(const PhysicalAddress& other) const -> bool { return Value < other.Value; }

auto PhysicalAddress::operator>(const PhysicalAddress& other) const -> bool { return Value > other.Value; }

}
