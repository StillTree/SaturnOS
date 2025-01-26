#include "Memory.hpp"

namespace SaturnKernel {

auto MemoryFill(void* ptr, u8 value, usize size) -> void
{
	u8* p = static_cast<u8*>(ptr);
	while (size > 0) {
		p[--size] = value;
	}
}

auto MemoryCopy(void* ptr1, void* ptr2, usize size) -> void
{
	u8* src = static_cast<u8*>(ptr1);
	u8* dest = static_cast<u8*>(ptr2);

	for (usize i = 0; i < size; i++) {
		dest[i] = src[i];
	}
}

auto MemoryCompare(const void* ptr1, const void* ptr2, usize size) -> bool
{
	const u8* a = static_cast<const u8*>(ptr1);
	const u8* b = static_cast<const u8*>(ptr2);

	for (usize i = 0; i < size; i++) {
		if (a[i] != b[i])
			return false;
	}

	return true;
}

}
