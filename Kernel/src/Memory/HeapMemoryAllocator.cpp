#include "Memory/HeapMemoryAllocator.hpp"

#include "Logger.hpp"
#include "Memory/BitmapFrameAllocator.hpp"
#include "Memory/Page.hpp"

namespace SaturnKernel {

HeapMemoryAllocator g_heapMemoryAllocator = {};

HeapMemoryAllocator::HeapMemoryAllocator()
	: m_head { .Size = 0, .Next = nullptr }
{
}

auto HeapMemoryAllocator::Init(USIZE heapSize, VirtualAddress heapBeginning) -> Result<void>
{
	const Page<Size4KiB> maxPage(heapBeginning.Value + heapSize);

	heapSize &= ~0xfff;

	for (Page<Size4KiB> heapPage(heapBeginning); heapPage <= maxPage; heapPage++) {
		auto frame = g_frameAllocator.AllocateFrame();

		if (frame.IsError()) {
			SK_LOG_ERROR("An unexpected error occured while trying to allocate a memory frame for the kernel's heap");
			return Result<void>::MakeErr(frame.Error);
		}

		frame.Value.MapTo(heapPage, PageTableEntryFlags::Present | PageTableEntryFlags::Writeable);
	}

	auto* wholeHeap = reinterpret_cast<HeapBlockHeader*>(heapBeginning.Value);
	wholeHeap->Size = heapSize;
	wholeHeap->Next = nullptr;

	m_head.Next = wholeHeap;

	return Result<void>::MakeOk();
}

auto HeapMemoryAllocator::AddFreeRegion(U64 address, USIZE size) -> Result<void>
{
	if (size < sizeof(HeapBlockHeader))
		return Result<void>::MakeErr(ErrorCode::SerialOutputUnavailabe);

	if ((address & (alignof(HeapBlockHeader) - 1)) != 0)
		return Result<void>::MakeErr(ErrorCode::SerialOutputUnavailabe);

	auto* newBlock = reinterpret_cast<HeapBlockHeader*>(address);
	newBlock->Size = size;
	newBlock->Next = m_head.Next;

	m_head.Next = newBlock;
	m_head.Size = 0;

	return Result<void>::MakeOk();
}

auto HeapMemoryAllocator::Allocate(USIZE size, USIZE alignment) -> Result<void*>
{
	size = size < sizeof(HeapBlockHeader) ? sizeof(HeapBlockHeader) : size;
	alignment = alignment < alignof(HeapBlockHeader) ? alignof(HeapBlockHeader) : alignment;

	HeapBlockHeader* currentHeader = &m_head;

	while (currentHeader->Next != nullptr) {
		U64 blockAddress = reinterpret_cast<U64>(currentHeader->Next);

		U64 alignedAddress = (blockAddress + alignment - 1) & ~(alignment - 1);
		USIZE alignmentOffset = alignedAddress - blockAddress;
		USIZE neededSize = size + alignmentOffset;

		if (currentHeader->Next->Size >= neededSize) {
			// If an allocation is gonna leave enough space for another block, create it
			if (currentHeader->Next->Size >= neededSize + sizeof(HeapBlockHeader)) {
				auto result = SplitBlock(currentHeader->Next, neededSize);
				if (result.IsError())
					return Result<void*>::MakeErr(result.Error);
			}

			currentHeader->Next = currentHeader->Next->Next;

			// Not doing that, would leave a gap of forever unusable memory in the heap
			if(alignmentOffset >= sizeof(HeapBlockHeader))
				AddFreeRegion(blockAddress, alignmentOffset);

			return Result<void*>::MakeOk(alignedAddress);
		}

		currentHeader = currentHeader->Next;
	}

	return Result<void*>::MakeErr(ErrorCode::OutOfMemory);
}

auto HeapMemoryAllocator::SplitBlock(HeapBlockHeader* block, USIZE newSize) -> Result<void>
{
	if (newSize >= block->Size)
		return Result<void>::MakeErr(ErrorCode::SerialOutputUnavailabe);

	if (newSize < sizeof(HeapBlockHeader))
		return Result<void>::MakeErr(ErrorCode::SerialOutputUnavailabe);

	newSize = (newSize + alignof(HeapBlockHeader) - 1) & ~(alignof(HeapBlockHeader) - 1);

	USIZE sizeDifference = block->Size - newSize;

	if (sizeDifference < sizeof(HeapBlockHeader))
		return Result<void>::MakeErr(ErrorCode::SerialOutputUnavailabe);

	block->Size = newSize;

	auto* newBlock = reinterpret_cast<HeapBlockHeader*>(reinterpret_cast<U64>(block) + block->Size);
	newBlock->Size = sizeDifference;
	newBlock->Next = block->Next;
	block->Next = newBlock;

	return Result<void>::MakeOk();
}

auto HeapMemoryAllocator::PrintHeaders() -> void
{
	auto* currentHeader = &m_head;

	while (currentHeader != nullptr) {
		SK_LOG_INFO("Size: {} Next: {}", currentHeader->Size, reinterpret_cast<U64>(currentHeader->Next));

		currentHeader = currentHeader->Next;
	}
}

auto HeapMemoryAllocator::Deallocate(VirtualAddress blockAddress, USIZE size) -> Result<void>
{
	size = size < sizeof(HeapBlockHeader) ? sizeof(HeapBlockHeader) : size;

	return AddFreeRegion(blockAddress.Value, size);
}

}
