#pragma once

#include "Core.h"
#include "Memory/PhysAddr.h"
#include "Memory/VirtAddr.h"

typedef struct MemoryMapEntry {
	PhysAddr PhysicalStart;
	PhysAddr PhysicalEnd;
} MemoryMapEntry;

/// C's memset but without a shitty name.
void MemoryFill(void* ptr, u8 value, usz size);
/// C's memcpy but without a shitty name.
void MemoryCopy(void* source, void* destination, usz size);
/// Returns `true` if the memory regions are the same and `false` when they're not.
bool MemoryCompare(const void* ptr1, const void* ptr2, usz size);
/// Returns the size of the given null-terminated string.
usz StringSize(const i8* string);

/// Invalidates the whole TLB cache by reloading the CR3 register.
inline void FlushTLB()
{
	u64 pml4Address = 0;
	__asm__ volatile("mov %%cr3, %0" : "=r"(pml4Address));
	__asm__ volatile("mov %0, %%cr3" : : "r"(pml4Address) : "memory");
}

/// Invalidates a memory page which contains the provided virtual address by using the `invlpg` instruction.
static inline void FlushPage(VirtAddr address) { __asm__ volatile("invlpg (%0)" : : "r"(address) : "memory"); }
