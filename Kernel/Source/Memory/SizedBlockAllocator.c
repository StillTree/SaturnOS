#include "Memory/SizedBlockAllocator.h"

#include "Logger.h"
#include "Memory.h"

static void SizedBlockSetStatus(SizedBlockAllocator* blockAllocator, usz index, bool used)
{
	if (index >= blockAllocator->AllocationCapacity) {
		return;
	}

	const usz mapIndex = index / 64;
	const usz bitIndex = index % 64;

	if (used) {
		blockAllocator->BlockBitmap[mapIndex] |= (1 << bitIndex);
	} else {
		blockAllocator->BlockBitmap[mapIndex] &= ~(1 << bitIndex);
	}
}

bool SizedBlockGetStatus(SizedBlockAllocator* blockAllocator, usz index)
{
	if (index >= blockAllocator->AllocationCapacity) {
		return false;
	}

	const usz mapIndex = index / 64;
	const usz bitIndex = index % 64;

	return ((blockAllocator->BlockBitmap[mapIndex] >> bitIndex) & 1) == 1;
}

Result InitSizedBlockAllocator(SizedBlockAllocator* blockAllocator, void* poolStart, usz poolSizeBytes, usz blockSizeBytes)
{
	// TODO: Use more efficient bitscanning (compiler intrinsics)
	blockAllocator->BlockSizeBytes = blockSizeBytes;
	blockAllocator->PoolSizeBytes = poolSizeBytes;

	const usz totalBlockCapacity = poolSizeBytes / blockSizeBytes;
	const usz bitmapWordCount = (totalBlockCapacity + 63) / 64;
	const usz blocksTakenUp = (bitmapWordCount * 8 + blockSizeBytes - 1) / blockSizeBytes;

	blockAllocator->BlockBitmap = poolStart;
	// The allocator expects the blocks to be correctly padded for alignment,
	// if they're not, some bad things are probably gonna happen
	blockAllocator->FirstBlock = (u8*)poolStart + blocksTakenUp * blockSizeBytes;
	blockAllocator->LastBlock = (u8*)poolStart + poolSizeBytes - blockSizeBytes;
	blockAllocator->AllocationCapacity = totalBlockCapacity - blocksTakenUp;
	blockAllocator->AllocationCount = 0;
	blockAllocator->NextToAllocate = 0;

	MemoryFill(blockAllocator->BlockBitmap, 0, bitmapWordCount * 8);

	return ResultOk;
}

Result SizedBlockAllocate(SizedBlockAllocator* blockAllocator, void** block)
{
	for (usz i = blockAllocator->NextToAllocate; i < blockAllocator->AllocationCapacity; ++i) {
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
	if ((u8*)block < blockAllocator->FirstBlock || (u8*)block > blockAllocator->LastBlock) {
		return ResultSerialOutputUnavailable;
	}

	usz index = SizedBlockGetIndex(blockAllocator, block);
	bool allocated = SizedBlockGetStatus(blockAllocator, index);

	if (!allocated) {
		LogLine(SK_LOG_WARN "An attempt was made to deallocate an unallocated memory block");
		return ResultSerialOutputUnavailable;
	}

	SizedBlockSetStatus(blockAllocator, index, false);

	blockAllocator->AllocationCount--;
	blockAllocator->NextToAllocate = 0;

	return ResultOk;
}

Result SizedBlockIterate(SizedBlockAllocator* blockAllocator, void** sizedBlockIterator)
{
	if (*sizedBlockIterator) {
		*sizedBlockIterator = (u8*)*sizedBlockIterator + blockAllocator->BlockSizeBytes;
	} else {
		*sizedBlockIterator = blockAllocator->FirstBlock;
	}

	for (usz i = SizedBlockGetIndex(blockAllocator, *sizedBlockIterator); i < blockAllocator->AllocationCapacity; ++i) {
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

	if (*sizedBlockIterator) {
		*sizedBlockIterator = (u8*)*sizedBlockIterator + blockAllocator->BlockSizeBytes;
	} else {
		*sizedBlockIterator = blockAllocator->FirstBlock;
	}

	usz i = SizedBlockGetIndex(blockAllocator, *sizedBlockIterator);
	while (true) {
		if (i >= blockAllocator->AllocationCapacity) {
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
