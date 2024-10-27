#include "Memory.hpp"

namespace SaturnKernel
{
	auto MemoryFill(void* ptr, U8 value, USIZE size) -> void
	{
		U8* p = static_cast<U8*>(ptr);
		while(size > 0)
		{
			p[--size] = value;
		}
	}

	auto MemoryCopy(void* ptr1, void* ptr2, USIZE size) -> void
	{
		U8* src	 = static_cast<U8*>(ptr1);
		U8* dest = static_cast<U8*>(ptr2);

		for(USIZE i = 0; i < size; i++)
		{
			dest[i] = src[i];
		}
	}

	auto MemoryCompare(const void* ptr1, const void* ptr2, USIZE size) -> I32
	{
		const U8* a = static_cast<const U8*>(ptr1);
		const U8* b = static_cast<const U8*>(ptr2);

		for(USIZE i = 0; i < size; i++)
		{
			if(a[i] < b[i])
				return -1;

			if(a[i] > b[i])
				return 1;
		}

		return 0;
	}
}
