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

/// Initializes the virtual memory manager, expects a contiguous region of backed virtual memory.
Result InitVirtualMemoryAllocator(VirtualMemoryAllocator* allocator, void* listBeginning, usz listSize, Frame4KiB pml4);
/// Allocates the given amount of physical memory and maps it to a randomly chosen virtual memory region.
Result AllocateBackedVirtualMemory(VirtualMemoryAllocator* allocator, usz size, PageTableEntryFlags flags, void** allocatedPage);
/// Allocates the given amount of physical memory and maps it to the specified virtual address.
Result AllocateBackedVirtualMemoryAtAddress(VirtualMemoryAllocator* allocator, usz size, PageTableEntryFlags flags, Page4KiB pageBegin);
/// Deallocates the given amount of physical memory and unmaps it from its corresponding virtual memory region.
Result DeallocateBackedVirtualMemory(VirtualMemoryAllocator* allocator, void* allocatedMemory, usz size);
/// Maps the given amount of physical memory to a randomly chosen virtual memory region.
Result AllocateMMIORegion(VirtualMemoryAllocator* allocator, Frame4KiB begin, usz size, PageTableEntryFlags flags, void** mmioBegin);
/// Deallocates the given amount of virtual memory.
Result DeallocateMMIORegion(VirtualMemoryAllocator* allocator, void* mmioBegin, usz size);
/// Just marks the given virtual memory region as used, where `begin` is inclusive and `end` exclusive.
Result MarkVirtualMemoryUsed(VirtualMemoryAllocator* allocator, Page4KiB begin, Page4KiB end);
/// Just marks the given virtual memory region as unused, where `begin` is inclusive and `end` exclusive.
Result MarkVirtualMemoryUnused(VirtualMemoryAllocator* allocator, Page4KiB begin, Page4KiB end);
/// Unmaps the given memory region in the source allocator and maps it in the destination one.
Result ReallocateVirtualMemory(VirtualMemoryAllocator* allocatorSource, VirtualMemoryAllocator* allocatorDestination, usz size,
	PageTableEntryFlags flags, Page4KiB pageSource, Page4KiB pageDestination);

/// Populates the given number of the top kernel PML4's entries and initializes the memory manager.
Result InitKernelVirtualMemory(usz topPML4Entries, Page4KiB backingMemoryBegin, usz backingMemorySize);

extern VirtualMemoryAllocator g_kernelMemoryAllocator;
