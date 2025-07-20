#include "Memory.h"

void MemoryFill(void* ptr, u8 value, usz size)
{
	u8* p = (u8*)ptr;
	while (size > 0) {
		p[--size] = value;
	}
}

void memcpy(void* destination, void* source, usz size)
{
	MemoryCopy(source, destination, size);
}

void MemoryCopy(void* source, void* destination, usz size)
{
	u8* src = source;
	u8* dest = destination;

	for (usz i = 0; i < size; i++) {
		dest[i] = src[i];
	}
}

bool MemoryCompare(const void* ptr1, const void* ptr2, usz size)
{
	const u8* a = (const u8*)ptr1;
	const u8* b = (const u8*)ptr2;

	for (usz i = 0; i < size; i++) {
		if (a[i] != b[i]) {
			return false;
		}
	}

	return true;
}

usz StringSize(const i8* string)
{
	usz i = 0;
	while (string[i]) {
		i++;
	}

	return i;
}
