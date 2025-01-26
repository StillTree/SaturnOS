#include "Memory/HeapMemoryAllocator.h"

#include "Logger.h"
#include "Memory/BitmapFrameAllocator.h"
#include "Memory/Page.h"

namespace SaturnKernel {

HeapMemoryAllocator g_heapMemoryAllocator = {};

HeapMemoryAllocator::HeapMemoryAllocator()
	: m_head { .Size = 0, .Next = nullptr }
{
}

auto HeapMemoryAllocator::Init(usize heapSize, VirtualAddress heapBeginning) -> Result<void>
{
	const Page<Size4KiB> maxPage(heapBeginning + heapSize);

	heapSize &= ~0xfff;

	for (Page<Size4KiB> heapPage(heapBeginning); heapPage <= maxPage; heapPage++) {
		auto frame = g_frameAllocator.AllocateFrame();
		if (frame.IsError()) {
			SK_LOG_ERROR("An unexpected error occured while trying to allocate a memory frame for the kernel's heap");
			return Result<void>::MakeErr(frame.Error);
		}

		auto result = heapPage.MapTo(frame.Value, PageTableEntryFlags::Present | PageTableEntryFlags::Writeable);
		if(result.IsError()) {
			SK_LOG_ERROR("An unexpected error occured while trying to map a memory frame for the kernel's heap");
			return Result<void>::MakeErr(result.Error);
		}
	}

	auto* wholeHeap = reinterpret_cast<HeapBlockHeader*>(heapBeginning.Value);
	wholeHeap->Size = heapSize;
	wholeHeap->Next = nullptr;

	m_head.Next = wholeHeap;

	return Result<void>::MakeOk();
}

auto HeapMemoryAllocator::AddFreeRegion(VirtualAddress address, usize size) -> Result<void>
{
	if (size < sizeof(HeapBlockHeader))
		return Result<void>::MakeErr(ErrorCode::HeapBlockTooSmall);

	if ((address.Value & (alignof(HeapBlockHeader) - 1)) != 0)
		return Result<void>::MakeErr(ErrorCode::HeapBlockIncorrectAlignment);

	auto* newBlock = address.AsPointer<HeapBlockHeader>();
	newBlock->Size = size;
	newBlock->Next = m_head.Next;

	m_head.Next = newBlock;
	m_head.Size = 0;

	return Result<void>::MakeOk();
}

auto HeapMemoryAllocator::Allocate(usize size, usize alignment) -> Result<void*>
{
	// I ensure that the allocated size can be actually freed later
	// and that the later freed region's header is correctly aligned.
	size = size < sizeof(HeapBlockHeader) ? sizeof(HeapBlockHeader) : size;
	alignment = alignment < alignof(HeapBlockHeader) ? alignof(HeapBlockHeader) : alignment;

	HeapBlockHeader* currentHeader = &m_head;

	while (currentHeader->Next != nullptr) {
		u64 blockAddress = reinterpret_cast<u64>(currentHeader->Next);

		u64 alignedAddress = (blockAddress + alignment - 1) & ~(alignment - 1);
		usize alignmentOffset = alignedAddress - blockAddress;
		usize neededSize = size + alignmentOffset;

		if (currentHeader->Next->Size >= neededSize) {
			// If an allocation is gonna leave enough space for another block, create it
			if (currentHeader->Next->Size >= neededSize + sizeof(HeapBlockHeader)) {
				auto result = SplitBlock(currentHeader->Next, neededSize);
				if (result.IsError())
					return Result<void*>::MakeErr(result.Error);
			}

			currentHeader->Next = currentHeader->Next->Next;

			// Not doing that, would leave a gap of forever unusable memory in the heap
			if(alignmentOffset >= sizeof(HeapBlockHeader)) {
				auto result = AddFreeRegion(VirtualAddress(blockAddress), alignmentOffset);
				if (result.IsError())
					return Result<void*>::MakeErr(result.Error);
			}

			return Result<void*>::MakeOk(alignedAddress);
		}

		currentHeader = currentHeader->Next;
	}

	return Result<void*>::MakeErr(ErrorCode::OutOfMemory);
}

auto HeapMemoryAllocator::SplitBlock(HeapBlockHeader* block, usize newSize) -> Result<void>
{
	// I ensure that the new size is also correctly aligned so that this doesn't case any issues with neighbouring blocks
	newSize = (newSize + alignof(HeapBlockHeader) - 1) & ~(alignof(HeapBlockHeader) - 1);

	if (newSize + sizeof(HeapBlockHeader) > block->Size || newSize < sizeof(HeapBlockHeader))
		return Result<void>::MakeErr(ErrorCode::HeapBlockIncorrectSplitSize);

	usize sizeDifference = block->Size - newSize;

	block->Size = newSize;

	auto* newBlock = reinterpret_cast<HeapBlockHeader*>(reinterpret_cast<u64>(block) + block->Size);
	newBlock->Size = sizeDifference;
	newBlock->Next = block->Next;
	block->Next = newBlock;

	return Result<void>::MakeOk();
}

auto HeapMemoryAllocator::PrintHeaders() -> void
{
	auto* currentHeader = &m_head;

	while (currentHeader != nullptr) {
		SK_LOG_INFO("Size: {} Next: {}", currentHeader->Size, reinterpret_cast<u64>(currentHeader->Next));

		currentHeader = currentHeader->Next;
	}
}

auto HeapMemoryAllocator::Deallocate(VirtualAddress blockAddress, usize size) -> Result<void>
{
	size = size < sizeof(HeapBlockHeader) ? sizeof(HeapBlockHeader) : size;

	return AddFreeRegion(blockAddress, size);
}

}
