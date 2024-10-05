#pragma once

#include "Core.hpp"

namespace SaturnKernel
{
	/// C's memset but without a shitty name.
	void MemoryFill(void* ptr, U8 value, USIZE size);
	/// C's memcpy but without a shitty name.
	void MemoryCopy(void* ptr1, void* ptr2, USIZE size);
	/// C's memcmp but without a shitty name.
	I32 MemoryCompare(const void* ptr1, const void* ptr2, USIZE size);
}
