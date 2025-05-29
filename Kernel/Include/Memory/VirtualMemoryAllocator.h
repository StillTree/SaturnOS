#pragma once

#include "Memory/Frame.h"
#include "Memory/Page.h"
#include "Memory/SizedBlockAllocator.h"
#include "Memory/VirtualAddress.h"
#include "Result.h"

typedef struct UnusedVirtualRegion {
	/// Inclusive
	VirtualAddress Begin;
	/// Exclusive
	VirtualAddress End;
	struct UnusedVirtualRegion* Previous;
	struct UnusedVirtualRegion* Next;
} UnusedVirtualRegion;

typedef struct VirtualMemoryAllocator {
	UnusedVirtualRegion* List;
	usz ListRegionCount;
	SizedBlockAllocator ListBackingStorage;
	Frame4KiB PML4;
} VirtualMemoryAllocator;

Result InitVirtualMemoryAllocator(VirtualMemoryAllocator* allocator, VirtualAddress listBeginning, usz listSize, Frame4KiB pml4);

Result AllocateBackedVirtualMemory(VirtualMemoryAllocator* allocator, usz size, PageTableEntryFlags flags, Page4KiB* allocatedPage);
Result DeallocateBackedVirtualMemory(VirtualMemoryAllocator* allocator, Page4KiB allocatedPage, usz size);
Result MarkVirtualMemoryUsed(VirtualMemoryAllocator* allocator, VirtualAddress begin, VirtualAddress end);
Result MarkVirtualMemoryUnused(VirtualMemoryAllocator* allocator, VirtualAddress begin, VirtualAddress end);

extern VirtualMemoryAllocator g_virtualMemoryAllocator;
