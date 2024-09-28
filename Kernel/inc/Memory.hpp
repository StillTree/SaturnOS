#pragma once

#include "Core.hpp"

namespace SaturnKernel
{
	void MemoryFill(void* ptr, U8 value, USIZE size);

	void MemoryCopy(void* ptr1, void* ptr2, USIZE size);

	I32 MemoryCompare(const void* ptr1, const void* ptr2, USIZE size);
}
