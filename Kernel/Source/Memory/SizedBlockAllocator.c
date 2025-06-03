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

static bool GetBlockStatus(SizedBlockAllocator* blockAllocator, VirtualAddress block)
{
	const usz blockIndex = (block - blockAllocator->FirstBlock) / blockAllocator->BlockSizeBytes;
	const usz mapIndex = (blockIndex) / 8;
	const usz bitIndex = blockIndex % 8;

	return ((blockAllocator->BlockBitmap[mapIndex] & (1 << bitIndex)) >> bitIndex) == 1;
}

Result InitSizedBlockAllocator(SizedBlockAllocator* blockAllocator, VirtualAddress poolStart, usz poolSizeBytes, usz blockSizeBytes)
{
	// TODO: Checks for correctness
	// TODO: Optimize using the last-allocated method
	blockAllocator->BlockSizeBytes = blockSizeBytes;
	blockAllocator->PoolSizeBytes = poolSizeBytes;

	const usz totalBlockCapacity = poolSizeBytes / blockSizeBytes;
	const usz bitmapSizeBytes = (totalBlockCapacity + 7) / 8;
	const usz blocksTakenUp = (bitmapSizeBytes + blockSizeBytes - 1) / blockSizeBytes;
	blockAllocator->BlockBitmap = (u8*)poolStart;
	blockAllocator->FirstBlock = poolStart + blocksTakenUp * blockSizeBytes;
	blockAllocator->LastBlock = poolStart + poolSizeBytes - blockSizeBytes;

	MemoryFill(blockAllocator->BlockBitmap, 0, bitmapSizeBytes);

	return ResultOk;
}

Result AllocateSizedBlock(SizedBlockAllocator* blockAllocator, void** block)
{
	for (VirtualAddress checkedBlock = blockAllocator->FirstBlock; checkedBlock <= blockAllocator->LastBlock;
		checkedBlock += blockAllocator->BlockSizeBytes) {
		if (GetBlockStatus(blockAllocator, checkedBlock)) {
			continue;
		}

		SetBlockStatus(blockAllocator, checkedBlock, true);
		*block = (void*)checkedBlock;
		return ResultOk;
	}

	return ResultOutOfMemory;
}

Result DeallocateSizedBlock(SizedBlockAllocator* blockAllocator, void* block)
{
	bool allocated = GetBlockStatus(blockAllocator, (VirtualAddress)block);

	if (!allocated) {
		SK_LOG_WARN("An attempt was made to deallocate an unallocated memory block");
		return ResultSerialOutputUnavailable;
	}

	SetBlockStatus(blockAllocator, (VirtualAddress)block, false);

	return ResultOk;
}
