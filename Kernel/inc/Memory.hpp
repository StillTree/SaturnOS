#pragma once

#include "Core.hpp"

namespace SaturnKernel
{
	/// C's memset but without a shitty name.
	void MemoryFill(void* ptr, U8 value, USIZE size);
	/// C's memcpy but without a shitty name.
	void MemoryCopy(void* ptr1, void* ptr2, USIZE size);
	/// C's memcmp but without a shitty name.
	auto MemoryCompare(const void* ptr1, const void* ptr2, USIZE size) -> I32;

	/// Returns the starting address for a physical frame that contains the given address
	/// (literally just aligns the address to the lower 4096 byte).
	auto PhysFrameContainingAddress(U64 address) -> U64;
}
