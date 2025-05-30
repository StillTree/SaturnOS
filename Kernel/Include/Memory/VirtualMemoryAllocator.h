#pragma once

#include "Memory/Frame.h"
#include "Memory/Page.h"
#include "Memory/SizedBlockAllocator.h"
#include "Memory/VirtualAddress.h"
#include "Result.h"

typedef struct UnusedVirtualRegion {
	/// Inclusive
	Page4KiB Begin;
	/// Exclusive
	Page4KiB End;
	struct UnusedVirtualRegion* Previous;
	struct UnusedVirtualRegion* Next;
} UnusedVirtualRegion;

typedef struct VirtualMemoryAllocator {
	UnusedVirtualRegion* List;
	SizedBlockAllocator ListBackingStorage;
	Frame4KiB PML4;
} VirtualMemoryAllocator;

Result InitVirtualMemoryAllocator(VirtualMemoryAllocator* allocator, Page4KiB listBeginning, usz listSize, Frame4KiB pml4);

Result AllocateBackedVirtualMemory(VirtualMemoryAllocator* allocator, usz size, PageTableEntryFlags flags, Page4KiB* allocatedPage);
Result DeallocateBackedVirtualMemory(VirtualMemoryAllocator* allocator, Page4KiB allocatedPage, usz size);

Result AllocateMMIORegion(VirtualMemoryAllocator* allocator, Frame4KiB begin, usz size, PageTableEntryFlags flags, Page4KiB* mmioBegin);
Result DeallocateMMIORegion(VirtualMemoryAllocator* allocator, Page4KiB begin, usz size);

Result MarkVirtualMemoryUsed(VirtualMemoryAllocator* allocator, Page4KiB begin, Page4KiB end);
Result MarkVirtualMemoryUnused(VirtualMemoryAllocator* allocator, Page4KiB begin, Page4KiB end);

extern VirtualMemoryAllocator g_virtualMemoryAllocator;
