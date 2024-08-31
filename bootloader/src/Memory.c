#include "Memory.h"

VOID MemoryFill(VOID* ptr, UINT8 value, UINTN size)
{
	UINT8* p = ptr;
	while(size > 0)
	{
		p[--size] = value;
	}
}
