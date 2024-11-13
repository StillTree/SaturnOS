#include "Memory/Page.hpp"

namespace SaturnKernel {

Page<Size4KiB>::Page(U64 address)
	: Address(address & ~0xfff)
{
}

Page<Size4KiB>::Page(VirtualAddress address)
	: Address(address.Address & ~0xfff)
{
}

auto Page<Size4KiB>::operator++(int) -> Page
{
	Page temp = *this;
	Address += SIZE_BYTES;
	return temp;
}

auto Page<Size4KiB>::operator--(int) -> Page
{
	Page temp = *this;
	Address -= SIZE_BYTES;
	return temp;
}

}
