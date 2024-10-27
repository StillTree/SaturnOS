#include "Memory/Frame.hpp"

namespace SaturnKernel
{
	Frame<Size4KiB>::Frame(U64 address)
		: Address(address & ~0xfff)
	{
	}

	auto Frame<Size4KiB>::operator++(int) -> Frame
	{
		Frame temp = *this;
		Address	  += SIZE_BYTES;
		return temp;
	}

	auto Frame<Size4KiB>::operator--(int) -> Frame
	{
		Frame temp = *this;
		Address	  -= SIZE_BYTES;
		return temp;
	}
	
	auto Frame<Size4KiB>::operator<(const Frame& other) const -> bool
	{
		return Address < other.Address;
	}

	auto Frame<Size4KiB>::operator>(const Frame& other) const -> bool
	{
		return Address > other.Address;
	}

	auto Frame<Size4KiB>::operator+(U64 other) const -> Frame
	{
		return Frame(Address + (other * 4096));
	}

	auto Frame<Size4KiB>::operator-(U64 other) const -> Frame
	{
		return Frame(Address - (other * 4096));
	}

	auto Frame<Size4KiB>::MapTo(const Page<Size4KiB>& page) -> Result<void>
	{
		return Result<void>::MakeOk();
	}
}
