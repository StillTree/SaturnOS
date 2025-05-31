#include "Memory/VirtualMemoryAllocator.h"

#include "Core.h"
#include "Logger.h"
#include "Memory.h"
#include "Memory/BitmapFrameAllocator.h"
#include "Memory/Frame.h"
#include "Memory/Page.h"
#include "Memory/PageTable.h"
#include "Random.h"

VirtualMemoryAllocator g_virtualMemoryAllocator = {};

static Result GetContainingRegion(VirtualMemoryAllocator* allocator, Page4KiB begin, Page4KiB end, UnusedVirtualRegion** containingRegion)
{
	if ((begin & 0xfff) != 0 || (end & 0xfff) != 0) {
		return ResultInvalidPageAlignment;
	}

	UnusedVirtualRegion* regionPointer = allocator->List;
	while (regionPointer) {
		if (begin >= regionPointer->Begin && end <= regionPointer->End) {
			*containingRegion = regionPointer;
			return ResultOk;
		}

		regionPointer = regionPointer->Next;
	}

	return ResultNotFound;
}

static Result GetBorderingBegin(VirtualMemoryAllocator* allocator, Page4KiB begin, UnusedVirtualRegion** borderingRegion)
{
	if ((begin & 0xfff) != 0) {
		return ResultInvalidPageAlignment;
	}

	UnusedVirtualRegion* regionPointer = allocator->List;
	while (regionPointer) {
		if (regionPointer->End == begin) {
			*borderingRegion = regionPointer;
			return ResultOk;
		}

		regionPointer = regionPointer->Next;
	}

	return ResultNotFound;
}

static Result GetBorderingEnd(VirtualMemoryAllocator* allocator, Page4KiB end, UnusedVirtualRegion** borderingRegion)
{
	if ((end & 0xfff) != 0) {
		return ResultInvalidPageAlignment;
	}

	UnusedVirtualRegion* regionPointer = allocator->List;
	while (regionPointer) {
		if (regionPointer->Begin == end) {
			*borderingRegion = regionPointer;
			return ResultOk;
		}

		regionPointer = regionPointer->Next;
	}

	return ResultNotFound;
}

static Result GetRandomRegion(VirtualMemoryAllocator* allocator, usz size, Page4KiB* randomPage)
{
	if ((size & 0xfff) != 0) {
		return ResultInvalidPageAlignment;
	}

	usz regionCount = 0;
	UnusedVirtualRegion* regionPointer = allocator->List;
	while (regionPointer) {
		if (regionPointer->End - regionPointer->Begin >= size) {
			regionCount++;
		}

		regionPointer = regionPointer->Next;
	}

	if (regionCount == 0) {
		SK_LOG_WARN(
			"Could not find any remaining unused virtual memory regions of suitable size for 0x%x bytes, too much memory usage", size);
		return ResultOutOfMemory;
	}

	usz randomIndex = Random() % regionCount;

	regionPointer = allocator->List;
	while (regionPointer) {
		if (regionPointer->End - regionPointer->Begin >= size) {
			if (randomIndex == 0) {
				usz maxOffset = regionPointer->End - regionPointer->Begin - size;

				usz pageSlotCount = (maxOffset / PAGE_4KIB_SIZE_BYTES) + 1;
				usz offsetPages = 0;
				if (pageSlotCount > 1) {
					offsetPages = Random() % pageSlotCount;
				}

				*randomPage = regionPointer->Begin + offsetPages * PAGE_4KIB_SIZE_BYTES;
				SK_LOG_DEBUG("Randomly chosen page = 0x%x", *randomPage);
				return ResultOk;
			}

			randomIndex--;
		}

		regionPointer = regionPointer->Next;
	}

	return ResultOutOfMemory;
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

	// Exclue non-canonical virtual addresses
	result = MarkVirtualMemoryUsed(allocator, 0x0000800000000000, 0xFFFF800000000000);
	if (result) {
		return result;
	}

	// Exclude this list's backing storage
	result = MarkVirtualMemoryUsed(allocator, listBeginning, listBeginning + listSize);
	if (result) {
		return result;
	}

	// Exclude the identity mapped memory at an offset
	result = MarkVirtualMemoryUsed(
		allocator, g_bootInfo.PhysicalMemoryOffset, g_bootInfo.PhysicalMemoryOffset + g_bootInfo.PhysicalMemoryMappingSize);
	if (result) {
		return result;
	}

	// Exclude memory currently occupied by the kernel
	result = MarkVirtualMemoryUsed(allocator, g_bootInfo.KernelAddress, g_bootInfo.KernelAddress + g_bootInfo.KernelSize);
	if (result) {
		return result;
	}

	// TODO: Not assume that the framebuffer is perfectly 4096 bytes aligned
	// Exclude the framebuffer
	result = MarkVirtualMemoryUsed(
		allocator, (VirtualAddress)g_bootInfo.Framebuffer, (VirtualAddress)g_bootInfo.Framebuffer + g_bootInfo.FramebufferSize + 4096);
	if (result) {
		return result;
	}

	return result;
}

Result AllocateBackedVirtualMemory(VirtualMemoryAllocator* allocator, usz size, PageTableEntryFlags flags, Page4KiB* allocatedPage)
{
	if ((size & 0xfff) != 0) {
		return ResultInvalidPageAlignment;
	}

	Page4KiB pageBegin;
	Result result = GetRandomRegion(allocator, size, &pageBegin);
	if (result) {
		return result;
	}

	const Page4KiB endPage = pageBegin + size;
	for (Page4KiB page = pageBegin; page < endPage; page += PAGE_4KIB_SIZE_BYTES) {
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

	if ((allocatedPage & 0xfff) != 0 || (size & 0xfff) != 0) {
		return ResultInvalidPageAlignment;
	}

	const Page4KiB endPage = allocatedPage + size;
	for (Page4KiB page = allocatedPage; page < endPage; page += PAGE_4KIB_SIZE_BYTES) {
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

Result AllocateMMIORegion(VirtualMemoryAllocator* allocator, Frame4KiB begin, usz size, PageTableEntryFlags flags, Page4KiB* mmioBegin)
{
	if ((begin & 0xfff) != 0 || (size & 0xfff) != 0) {
		return ResultInvalidPageAlignment;
	}

	Page4KiB pageBegin;
	Result result = GetRandomRegion(allocator, size, &pageBegin);
	if (result) {
		return result;
	}

	const Page4KiB endPage = pageBegin + size;
	Frame4KiB frame = begin;
	for (Page4KiB page = pageBegin; page < endPage; page += PAGE_4KIB_SIZE_BYTES) {
		result = Page4KiBMapTo(PhysicalAddressAsPointer(allocator->PML4), page, frame, flags);
		if (result) {
			return result;
		}

		frame += PAGE_4KIB_SIZE_BYTES;
	}

	result = MarkVirtualMemoryUsed(allocator, pageBegin, endPage);
	if (result) {
		return result;
	}

	*mmioBegin = pageBegin;
	return result;
}

Result DeallocateMMIORegion(VirtualMemoryAllocator* allocator, Page4KiB begin, usz size)
{
	Result result = ResultOk;

	if ((begin & 0xfff) != 0 || (size & 0xfff) != 0) {
		return ResultInvalidPageAlignment;
	}

	const Page4KiB endPage = begin + size;
	for (Page4KiB page = begin; page < endPage; page += PAGE_4KIB_SIZE_BYTES) {
		result = Page4KiBUnmap(page);
		if (result) {
			return result;
		}

		FlushPage(page);
	}

	result = MarkVirtualMemoryUnused(allocator, begin, endPage);
	if (result) {
		return result;
	}

	return result;
}

static Result RemoveRegion(VirtualMemoryAllocator* allocator, UnusedVirtualRegion* region)
{
	if (region->Previous) {
		region->Previous->Next = region->Next;
	} else {
		allocator->List = region->Next;
	}

	if (region->Next) {
		region->Next->Previous = region->Previous;
	}

	Result result = DeallocateSizedBlock(&allocator->ListBackingStorage, region);
	if (result) {
		return result;
	}

	return result;
}

static Result AddRegion(VirtualMemoryAllocator* allocator, Page4KiB begin, Page4KiB end)
{
	UnusedVirtualRegion* region = nullptr;
	Result result = AllocateSizedBlock(&allocator->ListBackingStorage, (void**)&region);
	if (result) {
		return result;
	}

	region->Begin = begin;
	region->End = end;

	region->Next = allocator->List;
	region->Previous = nullptr;
	if (allocator->List) {
		allocator->List->Previous = region;
	}
	allocator->List = region;

	return result;
}

Result MarkVirtualMemoryUsed(VirtualMemoryAllocator* allocator, Page4KiB begin, Page4KiB end)
{
	UnusedVirtualRegion* containingRegion = nullptr;
	Result result = GetContainingRegion(allocator, begin, end, &containingRegion);
	if (result) {
		return result;
	}

	if (begin > containingRegion->Begin) {
		result = AddRegion(allocator, containingRegion->Begin, begin);
		if (result) {
			return result;
		}
	}

	if (end < containingRegion->End) {
		result = AddRegion(allocator, end, containingRegion->End);
		if (result) {
			return result;
		}
	}

	result = RemoveRegion(allocator, containingRegion);
	if (result) {
		return result;
	}

	return result;
}

Result MarkVirtualMemoryUnused(VirtualMemoryAllocator* allocator, Page4KiB begin, Page4KiB end)
{
	UnusedVirtualRegion* region;
	Result result = GetBorderingBegin(allocator, begin, &region);
	if (!result) {
		begin = region->Begin;

		result = RemoveRegion(allocator, region);
		if (result) {
			return result;
		}
	}

	result = GetBorderingEnd(allocator, end, &region);
	if (!result) {
		end = region->End;

		result = RemoveRegion(allocator, region);
		if (result) {
			return result;
		}
	}

	result = AddRegion(allocator, begin, end);
	if (result) {
		return result;
	}

	return result;
}
