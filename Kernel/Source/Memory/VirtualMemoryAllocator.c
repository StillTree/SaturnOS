#include "Memory/VirtualMemoryAllocator.h"

#include "Core.h"
#include "Logger.h"
#include "Memory/BitmapFrameAllocator.h"
#include "Memory/Page.h"

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

void PrintList(VirtualMemoryAllocator* allocator)
{
	UnusedVirtualRegion* region = allocator->List;
	while (region) {
		SK_LOG_DEBUG("Begin = 0x%x End = 0x%x", region->Begin, region->End);

		region = region->Next;
	}
}

void PrintRegion(UnusedVirtualRegion* region) { SK_LOG_DEBUG("Begin = 0x%x End = 0x%x", region->Begin, region->End); }

Result InitVirtualMemoryAllocator(VirtualMemoryAllocator* allocator, VirtualAddress listBeginning, usz listSize)
{
	const Page4KiB maxPage = Page4KiBContainingAddress(listBeginning + listSize);

	for (Page4KiB heapPage = Page4KiBContainingAddress(listBeginning); heapPage <= maxPage; heapPage += PAGE_4KIB_SIZE_BYTES) {
		PhysicalAddress frame;
		Result result = AllocateFrame(&g_frameAllocator, &frame);
		if (result) {
			SK_LOG_ERROR("An unexpected error occured while trying to allocate a memory frame for the kernel's heap");
			return result;
		}

		PageTableEntry* kernelPML4 = PhysicalAddressAsPointer(KernelPageTable4Address());
		result = Page4KiBMapTo(kernelPML4, heapPage, frame, PageWriteable);
		if (result) {
			SK_LOG_ERROR("An unexpected error occured while trying to map a memory frame for the kernel's heap");
			return result;
		}
	}

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

	result = AllocateVirtualMemory(allocator, listBeginning, listBeginning + listSize);
	if (result) {
		return result;
	}

	result = AllocateVirtualMemory(
		allocator, g_bootInfo.PhysicalMemoryOffset, g_bootInfo.PhysicalMemoryOffset + g_bootInfo.PhysicalMemoryMappingSize);
	if (result) {
		return result;
	}

	result = AllocateVirtualMemory(allocator, g_bootInfo.KernelAddress, g_bootInfo.KernelAddress + g_bootInfo.KernelSize);
	if (result) {
		return result;
	}

	// TODO: Not assume that the framebuffer is perfectly 4096 bytes aligned
	result = AllocateVirtualMemory(
		allocator, (VirtualAddress)g_bootInfo.Framebuffer, (VirtualAddress)g_bootInfo.Framebuffer + g_bootInfo.FramebufferSize + 4096);
	if (result) {
		return result;
	}

	PrintList(allocator);

	return result;
}

Result AllocateVirtualMemory(VirtualMemoryAllocator* allocator, VirtualAddress begin, VirtualAddress end)
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

	return ResultOk;
}

Result DeallocateVirtualMemory(VirtualMemoryAllocator* allocator, VirtualAddress begin, VirtualAddress end)
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

	return result;
}
