#include "Memory/SizedBlockAllocator.h"

#include "Logger.h"
#include "Memory.h"

static void SizedBlockSetStatus(SizedBlockAllocator* blockAllocator, usz index, bool used)
{
	const usz mapIndex = (index) / 64;
	const usz bitIndex = index % 64;

	if (used) {
		blockAllocator->BlockBitmap[mapIndex] |= (1 << bitIndex);
	} else {
		blockAllocator->BlockBitmap[mapIndex] &= ~(1 << bitIndex);
	}
}

/// Returns `true` for allocated blocks and `false` for the unallocated ones.
static bool SizedBlockGetStatus(SizedBlockAllocator* blockAllocator, usz index)
{
	const usz mapIndex = (index) / 64;
	const usz bitIndex = index % 64;

	return ((blockAllocator->BlockBitmap[mapIndex] >> bitIndex) & 1) == 1;
}

static inline void* SizedBlockGetAddress(SizedBlockAllocator* blockAllocator, usz index)
{
	return (void*)(blockAllocator->FirstBlock + (blockAllocator->BlockSizeBytes * index));
}

static inline usz SizedBlockGetIndex(SizedBlockAllocator* blockAllocator, void* address)
{
	return ((VirtualAddress)address - blockAllocator->FirstBlock) / blockAllocator->BlockSizeBytes;
}

Result InitSizedBlockAllocator(SizedBlockAllocator* blockAllocator, VirtualAddress poolStart, usz poolSizeBytes, usz blockSizeBytes)
{
	// TODO: Use more efficient bitscanning (compiler intrinsics)
	blockAllocator->BlockSizeBytes = blockSizeBytes;
	blockAllocator->PoolSizeBytes = poolSizeBytes;

	const usz totalBlockCapacity = poolSizeBytes / blockSizeBytes;
	const usz bitmapWordCount = (totalBlockCapacity + 63) / 64;
	const usz blocksTakenUp = (bitmapWordCount * 8 + blockSizeBytes - 1) / blockSizeBytes;

	blockAllocator->BlockBitmap = (u64*)poolStart;
	// The allocator expects the blocks to be correctly padded for alignment,
	// if they're not, some bad things are probably gonna happen
	blockAllocator->FirstBlock = poolStart + blocksTakenUp * blockSizeBytes;
	blockAllocator->LastBlock = poolStart + poolSizeBytes - blockSizeBytes;
	blockAllocator->MaxAllocations = totalBlockCapacity - blocksTakenUp;
	blockAllocator->AllocationCount = 0;
	blockAllocator->NextToAllocate = 0;

	MemoryFill(blockAllocator->BlockBitmap, 0, bitmapWordCount * 8);

	return ResultOk;
}

Result SizedBlockAllocate(SizedBlockAllocator* blockAllocator, void** block)
{
	for (usz i = blockAllocator->NextToAllocate; i < blockAllocator->MaxAllocations; ++i) {
		if (SizedBlockGetStatus(blockAllocator, i)) {
			continue;
		}

		SizedBlockSetStatus(blockAllocator, i, true);
		blockAllocator->AllocationCount++;
		blockAllocator->NextToAllocate = i + 1;
		*block = SizedBlockGetAddress(blockAllocator, i);
		return ResultOk;
	}

	return ResultOutOfMemory;
}

Result SizedBlockDeallocate(SizedBlockAllocator* blockAllocator, void* block)
{
	VirtualAddress blockAddress = (VirtualAddress)block;
	if (blockAddress < blockAllocator->FirstBlock || blockAddress > blockAllocator->LastBlock) {
		return ResultSerialOutputUnavailable;
	}

	usz index = SizedBlockGetIndex(blockAllocator, block);
	bool allocated = SizedBlockGetStatus(blockAllocator, index);

	if (!allocated) {
		SK_LOG_WARN("An attempt was made to deallocate an unallocated memory block");
		return ResultSerialOutputUnavailable;
	}

	SizedBlockSetStatus(blockAllocator, index, false);

	blockAllocator->AllocationCount--;
	blockAllocator->NextToAllocate = 0;

	return ResultOk;
}

Result SizedBlockIterate(SizedBlockAllocator* blockAllocator, void** sizedBlockIterator)
{
	for (usz i = SizedBlockGetIndex(blockAllocator, *sizedBlockIterator); i < blockAllocator->MaxAllocations; ++i) {
		if (!SizedBlockGetStatus(blockAllocator, i)) {
			continue;
		}

		*sizedBlockIterator = SizedBlockGetAddress(blockAllocator, i);
		return ResultOk;
	}

	return ResultEndOfIteration;
}

Result SizedBlockCircularIterate(SizedBlockAllocator* blockAllocator, void** sizedBlockIterator)
{
	if (blockAllocator->AllocationCount == 0) {
		return ResultEndOfIteration;
	}

	usz i = SizedBlockGetIndex(blockAllocator, *sizedBlockIterator);
	while (true) {
		if (i >= blockAllocator->MaxAllocations) {
			i = 0;
			continue;
		}

		if (!SizedBlockGetStatus(blockAllocator, i)) {
			++i;
			continue;
		}

		*sizedBlockIterator = SizedBlockGetAddress(blockAllocator, i);
		return ResultOk;
	}
}
