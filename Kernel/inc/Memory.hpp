#pragma once

#include "Core.hpp"

namespace SaturnKernel
{
	struct Size4KiB
	{
	};
	struct Size2MiB
	{
	};
	struct Size1GiB
	{
	};

	/// C's memset but without a shitty name.
	auto MemoryFill(void* ptr, U8 value, USIZE size) -> void;
	/// C's memcpy but without a shitty name.
	auto MemoryCopy(void* ptr1, void* ptr2, USIZE size) -> void;
	/// C's memcmp but without a shitty name.
	auto MemoryCompare(const void* ptr1, const void* ptr2, USIZE size) -> I32;
}
