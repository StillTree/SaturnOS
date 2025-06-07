#include "Memory/SizedBlockAllocator.h"

#include "Logger.h"
#include "Memory.h"

static void SetBlockStatus(SizedBlockAllocator* blockAllocator, VirtualAddress block, bool used)
{
	const usz blockIndex = (block - blockAllocator->FirstBlock) / blockAllocator->BlockSizeBytes;
	const usz mapIndex = (blockIndex) / 8;
	const usz bitIndex = blockIndex % 8;

	if (used) {
		blockAllocator->BlockBitmap[mapIndex] |= (1 << bitIndex);
	} else {
		blockAllocator->BlockBitmap[mapIndex] &= ~(1 << bitIndex);
	}
}

Result InitSizedBlockAllocator(SizedBlockAllocator* blockAllocator, VirtualAddress poolStart, usz poolSizeBytes, usz blockSizeBytes)
{
	// TODO: Checks for correctness
	// TODO: Optimize using the last-allocated method
	// TODO: Use u64 (native word size) for the bitmap
	// TODO: Use more efficient bitscanning (compiler intrinsics)
	// TODO: Maybe some iterator function or something
	// TODO: Bruh... just rewrite that and optimise next time
	blockAllocator->BlockSizeBytes = blockSizeBytes;
	blockAllocator->PoolSizeBytes = poolSizeBytes;

	const usz totalBlockCapacity = poolSizeBytes / blockSizeBytes;
	const usz bitmapSizeBytes = (totalBlockCapacity + 7) / 8;
	const usz blocksTakenUp = (bitmapSizeBytes + blockSizeBytes - 1) / blockSizeBytes;
	blockAllocator->BlockBitmap = (u8*)poolStart;
	blockAllocator->FirstBlock = poolStart + blocksTakenUp * blockSizeBytes;
	blockAllocator->LastBlock = poolStart + poolSizeBytes - blockSizeBytes;
	blockAllocator->AllocationCount = 0;

	MemoryFill(blockAllocator->BlockBitmap, 0, bitmapSizeBytes);

	return ResultOk;
}

bool GetSizedBlockStatus(SizedBlockAllocator* blockAllocator, VirtualAddress block)
{
	const usz blockIndex = (block - blockAllocator->FirstBlock) / blockAllocator->BlockSizeBytes;
	const usz mapIndex = (blockIndex) / 8;
	const usz bitIndex = blockIndex % 8;

	return ((blockAllocator->BlockBitmap[mapIndex] & (1 << bitIndex)) >> bitIndex) == 1;
}

Result AllocateSizedBlock(SizedBlockAllocator* blockAllocator, void** block)
{
	for (VirtualAddress checkedBlock = blockAllocator->FirstBlock; checkedBlock <= blockAllocator->LastBlock;
		checkedBlock += blockAllocator->BlockSizeBytes) {
		if (GetSizedBlockStatus(blockAllocator, checkedBlock)) {
			continue;
		}

		SetBlockStatus(blockAllocator, checkedBlock, true);
		*block = (void*)checkedBlock;
		blockAllocator->AllocationCount++;
		return ResultOk;
	}

	return ResultOutOfMemory;
}

Result DeallocateSizedBlock(SizedBlockAllocator* blockAllocator, void* block)
{
	bool allocated = GetSizedBlockStatus(blockAllocator, (VirtualAddress)block);

	if (!allocated) {
		SK_LOG_WARN("An attempt was made to deallocate an unallocated memory block");
		return ResultSerialOutputUnavailable;
	}

	SetBlockStatus(blockAllocator, (VirtualAddress)block, false);

	blockAllocator->AllocationCount--;

	return ResultOk;
}
