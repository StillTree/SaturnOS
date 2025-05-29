#include "Memory/VirtualMemoryAllocator.h"

#include "Core.h"
#include "Memory.h"
#include "Memory/BitmapFrameAllocator.h"
#include "Memory/Frame.h"
#include "Memory/Page.h"
#include "Memory/PageTable.h"
#include <immintrin.h>

VirtualMemoryAllocator g_virtualMemoryAllocator = {};

static Result GetContainingRegion(
	VirtualMemoryAllocator* allocator, VirtualAddress begin, VirtualAddress end, UnusedVirtualRegion** containingRegion)
{
	UnusedVirtualRegion* region = allocator->List;
	while (region) {
		if (begin >= region->Begin && end <= region->End) {
			*containingRegion = region;
			return ResultOk;
		}

		region = region->Next;
	}

	return ResultSerialOutputUnavailabe;
}

static Result GetRandomRegion(VirtualMemoryAllocator* allocator, usz size, Page4KiB* randomPage)
{
	usz regionCount = 0;
	UnusedVirtualRegion* region = allocator->List;
	while (region) {
		if (region->End - region->Begin >= size) {
			regionCount++;
		}

		region = region->Next;
	}

	if (regionCount == 0) {
		return ResultSerialOutputUnavailabe;
	}

	usz randomIndex;
	while (!_rdrand64_step(&randomIndex))
		;
	randomIndex %= regionCount;

	region = allocator->List;
	while (region) {
		if (region->End - region->Begin >= size) {
			if (randomIndex == 0) {
				usz maxOffset = region->End - region->Begin - size;

				usz pageSlotCount = (maxOffset / PAGE_4KIB_SIZE_BYTES) + 1;
				usz offsetPages = 0;
				if (pageSlotCount > 1) {
					while (!_rdrand64_step(&offsetPages))
						;
					offsetPages %= pageSlotCount;
				}

				*randomPage = region->Begin + offsetPages * PAGE_4KIB_SIZE_BYTES;
				return ResultOk;
			}

			randomIndex--;
		}

		region = region->Next;
	}

	return ResultSerialOutputUnavailabe;
}

// void PrintList(VirtualMemoryAllocator* allocator)
// {
// 	UnusedVirtualRegion* region = allocator->List;
// 	while (region) {
// 		SK_LOG_DEBUG("Begin = 0x%x End = 0x%x", region->Begin, region->End);
//
// 		region = region->Next;
// 	}
// }

Result InitVirtualMemoryAllocator(VirtualMemoryAllocator* allocator, VirtualAddress listBeginning, usz listSize, Frame4KiB pml4)
{
	const Page4KiB endPage = Page4KiBContainingAddress(listBeginning + listSize);

	for (Page4KiB heapPage = Page4KiBContainingAddress(listBeginning); heapPage <= endPage; heapPage += PAGE_4KIB_SIZE_BYTES) {
		PhysicalAddress frame;
		Result result = AllocateFrame(&g_frameAllocator, &frame);
		if (result) {
			return result;
		}

		PageTableEntry* kernelPML4 = PhysicalAddressAsPointer(KernelPageTable4Address());
		result = Page4KiBMapTo(kernelPML4, heapPage, frame, PageWriteable);
		if (result) {
			return result;
		}
	}

	allocator->PML4 = pml4;

	Result result = InitSizedBlockAllocator(&allocator->ListBackingStorage, listBeginning, listSize, sizeof(UnusedVirtualRegion));
	if (result) {
		return result;
	}

	result = AllocateSizedBlock(&allocator->ListBackingStorage, (void**)&allocator->List);
	if (result) {
		return result;
	}

	// The first page should always be unused to catch bugs
	allocator->List->Begin = 4096;
	allocator->List->End = U64_MAX;
	allocator->List->Next = nullptr;
	allocator->List->Previous = nullptr;
	allocator->ListRegionCount = 1;

	result = MarkVirtualMemoryUsed(allocator, listBeginning, listBeginning + listSize);
	if (result) {
		return result;
	}

	result = MarkVirtualMemoryUsed(
		allocator, g_bootInfo.PhysicalMemoryOffset, g_bootInfo.PhysicalMemoryOffset + g_bootInfo.PhysicalMemoryMappingSize);
	if (result) {
		return result;
	}

	result = MarkVirtualMemoryUsed(allocator, g_bootInfo.KernelAddress, g_bootInfo.KernelAddress + g_bootInfo.KernelSize);
	if (result) {
		return result;
	}

	// TODO: Not assume that the framebuffer is perfectly 4096 bytes aligned
	result = MarkVirtualMemoryUsed(
		allocator, (VirtualAddress)g_bootInfo.Framebuffer, (VirtualAddress)g_bootInfo.Framebuffer + g_bootInfo.FramebufferSize + 4096);
	if (result) {
		return result;
	}

	return result;
}

Result AllocateBackedVirtualMemory(VirtualMemoryAllocator* allocator, usz size, PageTableEntryFlags flags, Page4KiB* allocatedPage)
{
	Result result = ResultOk;

	// TODO: Check if size is 4096-byte aligned

	Page4KiB pageBegin;
	result = GetRandomRegion(allocator, size, &pageBegin);
	if (result) {
		return result;
	}

	const Page4KiB endPage = pageBegin + size;
	for (Page4KiB page = pageBegin; page <= endPage; page += PAGE_4KIB_SIZE_BYTES) {
		PhysicalAddress frame;
		Result result = AllocateFrame(&g_frameAllocator, &frame);
		if (result) {
			return result;
		}

		result = Page4KiBMapTo(PhysicalAddressAsPointer(allocator->PML4), page, frame, flags);
		if (result) {
			return result;
		}
	}

	result = MarkVirtualMemoryUsed(allocator, pageBegin, endPage);
	if (result) {
		return result;
	}

	*allocatedPage = pageBegin;
	return result;
}

Result DeallocateBackedVirtualMemory(VirtualMemoryAllocator* allocator, Page4KiB allocatedPage, usz size)
{
	Result result = ResultOk;

	// TODO: Check if page and size are 4096-byte aligned

	const Page4KiB endPage = allocatedPage + size;
	for (Page4KiB page = allocatedPage; page <= endPage; page += PAGE_4KIB_SIZE_BYTES) {
		Frame4KiB frame;
		result = VirtualAddressToPhysical(page, PhysicalAddressAsPointer(allocator->PML4), &frame);
		if (result) {
			return result;
		}

		DeallocateFrame(&g_frameAllocator, frame);
		result = Page4KiBUnmap(page);
		if (result) {
			return result;
		}

		FlushPage(page);
	}

	result = MarkVirtualMemoryUnused(allocator, allocatedPage, endPage);
	if (result) {
		return result;
	}

	return result;
}

Result MarkVirtualMemoryUsed(VirtualMemoryAllocator* allocator, VirtualAddress begin, VirtualAddress end)
{
	UnusedVirtualRegion* containingRegion = nullptr;
	Result result = GetContainingRegion(allocator, begin, end, &containingRegion);
	if (result) {
		return result;
	}

	if (begin > containingRegion->Begin) {
		UnusedVirtualRegion* beginningRegion = nullptr;
		AllocateSizedBlock(&allocator->ListBackingStorage, (void**)&beginningRegion);

		beginningRegion->Begin = containingRegion->Begin;
		beginningRegion->End = begin;

		beginningRegion->Next = allocator->List;
		beginningRegion->Previous = nullptr;
		allocator->List->Previous = beginningRegion;
		allocator->List = beginningRegion;
		allocator->ListRegionCount++;
	}

	if (end < containingRegion->End) {
		UnusedVirtualRegion* endingRegion = nullptr;
		AllocateSizedBlock(&allocator->ListBackingStorage, (void**)&endingRegion);

		endingRegion->Begin = end;
		endingRegion->End = containingRegion->End;

		endingRegion->Next = allocator->List;
		endingRegion->Previous = nullptr;
		allocator->List->Previous = endingRegion;
		allocator->List = endingRegion;
		allocator->ListRegionCount++;
	}

	if (containingRegion->Previous) {
		containingRegion->Previous->Next = containingRegion->Next;
	} else {
		allocator->List = containingRegion->Next;
	}

	if (containingRegion->Next) {
		containingRegion->Next->Previous = containingRegion->Previous;
	}

	DeallocateSizedBlock(&allocator->ListBackingStorage, containingRegion);
	allocator->ListRegionCount--;

	return ResultOk;
}

Result MarkVirtualMemoryUnused(VirtualMemoryAllocator* allocator, VirtualAddress begin, VirtualAddress end)
{
	// TODO: Merge adjecent memory regions
	UnusedVirtualRegion* region = nullptr;
	Result result = AllocateSizedBlock(&allocator->ListBackingStorage, (void**)&region);
	if (result) {
		return result;
	}

	region->Begin = begin;
	region->End = end;
	region->Next = allocator->List;
	region->Previous = nullptr;
	allocator->List->Previous = region;
	allocator->List = region;
	allocator->ListRegionCount++;

	return result;
}
