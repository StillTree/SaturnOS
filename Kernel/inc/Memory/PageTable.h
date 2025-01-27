#pragma once

#include "Core.h"
#include "Memory/PhysicalAddress.h"

typedef enum PageTableEntryFlags : u64 {
	Present = 1ULL,
	Writeable = 1ULL << 1,
	UserAccessible = 1ULL << 2,
	WriteThroughCaching = 1ULL << 3,
	NoCache = 1ULL << 4,
	Accessed = 1ULL << 5,
	Dirty = 1ULL << 6,
	HugePage = 1ULL << 7,
	Global = 1ULL << 8,
	NoExecute = 1ULL << 63,
} PageTableEntryFlags;

typedef u64 PageTableEntry;

#define FRAME_ADDRESS_MASK (((1ULL << 40) - 1) << 12)
#define FLAGS_MASK 0xfff
#define PAGE_TABLE_ENTRIES 512

/// Returns the PML4 table's physical memory address, read from the CR3 register.
static inline PhysicalAddress PageTable4Address()
{
	PhysicalAddress pml4Address = -1;
	__asm__ volatile("mov %%cr3, %0" : "=r"(pml4Address));

	return pml4Address;
}
