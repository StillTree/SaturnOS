#pragma once

#include "Memory/Frame.h"
#include "Memory/Page.h"
#include "Memory/SizedBlockAllocator.h"
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

/// Initializes the virtual memory manager.
Result InitVirtualMemoryAllocator(VirtualMemoryAllocator* allocator, Page4KiB listBeginning, usz listSize, Frame4KiB pml4);

/// Allocates the given amount of physical memory and maps it to a randomly chosen virtual memory region.
Result AllocateBackedVirtualMemory(VirtualMemoryAllocator* allocator, usz size, PageTableEntryFlags flags, Page4KiB* allocatedPage);
/// Deallocates the given amount of physical memory and unmaps it from its corresponding virtual memory region.
Result DeallocateBackedVirtualMemory(VirtualMemoryAllocator* allocator, Page4KiB allocatedPage, usz size);

/// Maps the given amount of physical memory to a randomly chosen virtual memory region.
Result AllocateMMIORegion(VirtualMemoryAllocator* allocator, Frame4KiB begin, usz size, PageTableEntryFlags flags, Page4KiB* mmioBegin);
/// Deallocates the given amount of virtual memory.
Result DeallocateMMIORegion(VirtualMemoryAllocator* allocator, Page4KiB begin, usz size);

/// Just marks the given virtual memory region as used, where `begin` is inclusive and `end` exclusive.
Result MarkVirtualMemoryUsed(VirtualMemoryAllocator* allocator, Page4KiB begin, Page4KiB end);
/// Just marks the given virtual memory region as unused, where `begin` is inclusive and `end` exclusive.
Result MarkVirtualMemoryUnused(VirtualMemoryAllocator* allocator, Page4KiB begin, Page4KiB end);

extern VirtualMemoryAllocator g_virtualMemoryAllocator;
