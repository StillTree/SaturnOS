#include "Memory/VirtualMemoryAllocator.h"

#include "Core.h"
#include "Logger.h"
#include "Memory.h"
#include "Memory/BitmapFrameAllocator.h"
#include "Memory/Frame.h"
#include "Memory/Page.h"
#include "Memory/PageTable.h"
#include "Random.h"

VirtualMemoryAllocator g_kernelMemoryAllocator = {};

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
		Log(SK_LOG_WARN "Could not find any remaining unused virtual memory regions of suitable size for 0x%x bytes, too much memory usage",
			size);
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

Result InitKernelVirtualMemory(usz topPML4Entries, Page4KiB backingMemoryBegin, usz backingMemorySize)
{
	Result result = Page4KiBUnmap(g_bootInfo.ContextSwitchFunctionPage);
	if (result) {
		return result;
	}

	Frame4KiB kernelPML4Frame = KernelPML4();
	PageTableEntry* kernelPML4 = PhysicalAddressAsPointer(kernelPML4Frame);

	for (usz i = PAGE_TABLE_ENTRIES - 1; i > PAGE_TABLE_ENTRIES - 1 - topPML4Entries; i--) {
		if (kernelPML4[i] & PagePresent) {
			continue;
		}

		Frame4KiB frame;
		result = AllocateFrame(&g_frameAllocator, &frame);
		if (result) {
			return result;
		}

		InitEmptyPageTable(PhysicalAddressAsPointer(frame));

		kernelPML4[i] = frame | PagePresent | PageWriteable;
	}

	const Page4KiB endPage = Page4KiBContaining(backingMemoryBegin + backingMemorySize);

	for (Page4KiB heapPage = Page4KiBContaining(backingMemoryBegin); heapPage < endPage; heapPage += PAGE_4KIB_SIZE_BYTES) {
		PhysicalAddress frame;
		Result result = AllocateFrame(&g_frameAllocator, &frame);
		if (result) {
			return result;
		}

		PageTableEntry* kernelPML4 = PhysicalAddressAsPointer(KernelPML4());
		result = Page4KiBMapTo(kernelPML4, heapPage, frame, PageWriteable);
		if (result) {
			return result;
		}
	}

	result = InitVirtualMemoryAllocator(&g_kernelMemoryAllocator, backingMemoryBegin, backingMemorySize, kernelPML4Frame);
	if (result) {
		return result;
	}

	// Exclude everything up to the chosen amount of top PML4 entries
	result = MarkVirtualMemoryUsed(&g_kernelMemoryAllocator, 4096, 0xffff800000000000 + (0x8000000000 * (256 - topPML4Entries)));
	if (result) {
		return result;
	}

	// Exclude this list's backing storage
	result = MarkVirtualMemoryUsed(&g_kernelMemoryAllocator, backingMemoryBegin, backingMemoryBegin + backingMemorySize);
	if (result) {
		return result;
	}

	// Exclude memory currently occupied by the kernel
	result = MarkVirtualMemoryUsed(&g_kernelMemoryAllocator, g_bootInfo.KernelAddress, g_bootInfo.KernelAddress + g_bootInfo.KernelSize);
	if (result) {
		return result;
	}

	// TODO: Not assume that the framebuffer is perfectly 4096 bytes aligned
	// Exclude the framebuffer
	result = MarkVirtualMemoryUsed(&g_kernelMemoryAllocator, (VirtualAddress)g_bootInfo.Framebuffer,
		(VirtualAddress)g_bootInfo.Framebuffer + g_bootInfo.FramebufferSize);
	if (result) {
		return result;
	}

	// TODO: Not assume that the memory map's size is just 4096 bytes
	// Exclude the physical memory mapping and the physical memory map right after it
	result = MarkVirtualMemoryUsed(&g_kernelMemoryAllocator, g_bootInfo.PhysicalMemoryOffset,
		g_bootInfo.PhysicalMemoryOffset + g_bootInfo.PhysicalMemoryMappingSize + 0x1000);
	if (result) {
		return result;
	}

	return result;
}

Result InitVirtualMemoryAllocator(VirtualMemoryAllocator* allocator, VirtualAddress listBeginning, usz listSize, Frame4KiB pml4)
{
	allocator->PML4 = pml4;

	Result result = InitSizedBlockAllocator(&allocator->ListBackingStorage, listBeginning, listSize, sizeof(UnusedVirtualRegion));
	if (result) {
		return result;
	}

	result = SizedBlockAllocate(&allocator->ListBackingStorage, (void**)&allocator->List);
	if (result) {
		return result;
	}

	// The first page should always be unused to catch bugs
	// The last page is excluded since it's not possible to mark it as used,
	// because `End` is exclusive so the address would wrap around to 0
	allocator->List->Begin = PAGE_4KIB_SIZE_BYTES;
	allocator->List->End = U64_MAX - PAGE_4KIB_SIZE_BYTES + 1;
	allocator->List->Next = nullptr;
	allocator->List->Previous = nullptr;

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

	Result result = SizedBlockDeallocate(&allocator->ListBackingStorage, region);
	if (result) {
		return result;
	}

	return result;
}

static Result AddRegion(VirtualMemoryAllocator* allocator, Page4KiB begin, Page4KiB end)
{
	UnusedVirtualRegion* region = nullptr;
	Result result = SizedBlockAllocate(&allocator->ListBackingStorage, (void**)&region);
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
