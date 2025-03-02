#include "Memory/HeapMemoryAllocator.h"

#include "Logger.h"
#include "Memory/BitmapFrameAllocator.h"
#include "Memory/Page.h"

HeapMemoryAllocator g_heapMemoryAllocator = {};

Result HeapInit(HeapMemoryAllocator* heap, usz heapSize, VirtualAddress heapBeginning)
{
	const Page4KiB maxPage = Page4KiBContainingAddress(heapBeginning + heapSize);

	heapSize &= ~0xfff;

	for (Page4KiB heapPage = Page4KiBContainingAddress(heapBeginning); heapPage <= maxPage; heapPage += PAGE_4KIB_SIZE_BYTES) {
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

	HeapBlockHeader* wholeHeap = (HeapBlockHeader*)heapBeginning;
	wholeHeap->Size = heapSize;
	wholeHeap->Next = nullptr;

	heap->Head.Next = wholeHeap;

	return ResultOk;
}

static Result AddFreeRegion(HeapMemoryAllocator* heap, VirtualAddress address, usz size)
{
	if (size < sizeof(HeapBlockHeader))
		return ResultHeapBlockTooSmall;

	if (address & (alignof(HeapBlockHeader) - 1))
		return ResultHeapBlockIncorrectAlignment;

	HeapBlockHeader* newBlock = (HeapBlockHeader*)address;
	newBlock->Size = size;
	newBlock->Next = heap->Head.Next;

	heap->Head.Next = newBlock;
	heap->Head.Size = 0;

	return ResultOk;
}

static Result SplitBlock(HeapBlockHeader* block, usz newSize)
{
	// I ensure that the new size is also correctly aligned so that this doesn't case any issues with neighbouring blocks
	newSize = (newSize + alignof(HeapBlockHeader) - 1) & ~(alignof(HeapBlockHeader) - 1);

	if (newSize + sizeof(HeapBlockHeader) > block->Size || newSize < sizeof(HeapBlockHeader))
		return ResultHeapBlockIncorrectSplitSize;

	usz sizeDifference = block->Size - newSize;

	block->Size = newSize;

	HeapBlockHeader* newBlock = (HeapBlockHeader*)((u64)block + block->Size);
	newBlock->Size = sizeDifference;
	newBlock->Next = block->Next;
	block->Next = newBlock;

	return ResultOk;
}

Result HeapAllocate(HeapMemoryAllocator* heap, usz size, usz alignment, void** pointer)
{
	// I ensure that the allocated size can be actually freed later
	// and that the later freed region's header is correctly aligned.
	size = size < sizeof(HeapBlockHeader) ? sizeof(HeapBlockHeader) : size;
	alignment = alignment < alignof(HeapBlockHeader) ? alignof(HeapBlockHeader) : alignment;

	HeapBlockHeader* currentHeader = &heap->Head;

	while (currentHeader->Next != nullptr) {
		u64 blockAddress = (u64)currentHeader->Next;

		u64 alignedAddress = (blockAddress + alignment - 1) & ~(alignment - 1);
		usz alignmentOffset = alignedAddress - blockAddress;
		usz neededSize = size + alignmentOffset;

		if (currentHeader->Next->Size >= neededSize) {
			// If an allocation is gonna leave enough space for another block, create it
			if (currentHeader->Next->Size >= neededSize + sizeof(HeapBlockHeader)) {
				Result result = SplitBlock(currentHeader->Next, neededSize);
				if (result)
					return result;
			}

			currentHeader->Next = currentHeader->Next->Next;

			// Not doing that, would leave a gap of forever unusable memory in the heap
			if (alignmentOffset >= sizeof(HeapBlockHeader)) {
				Result result = AddFreeRegion(heap, blockAddress, alignmentOffset);
				if (result)
					return result;
			}

			*pointer = (void*)alignedAddress;
			return ResultOk;
		}

		currentHeader = currentHeader->Next;
	}

	return ResultOutOfMemory;
}

void HeapPrintHeaders(HeapMemoryAllocator* heap)
{
	HeapBlockHeader* currentHeader = &heap->Head;

	while (currentHeader != nullptr) {
		SK_LOG_INFO("Size: %u Next: %p", currentHeader->Size, currentHeader->Next);

		currentHeader = currentHeader->Next;
	}
}

Result HeapDeallocate(HeapMemoryAllocator* heap, VirtualAddress blockAddress, usz size)
{
	size = size < sizeof(HeapBlockHeader) ? sizeof(HeapBlockHeader) : size;

	return AddFreeRegion(heap, blockAddress, size);
}
