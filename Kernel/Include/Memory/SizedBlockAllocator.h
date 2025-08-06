#pragma once

#include "Core.h"
#include "Result.h"

typedef struct SizedBlockAllocator {
	/// Inclusive.
	u8* FirstBlock;
	/// Inclusive.
	u8* LastBlock;
	u64* BlockBitmap;
	usz PoolSizeBytes;
	usz BlockSizeBytes;
	usz AllocationCapacity;
	usz AllocationCount;
	usz NextToAllocate;
} SizedBlockAllocator;

/// Initializes the sized-block allocator. Expects a contiguous, mapped virtual memory region.
Result InitSizedBlockAllocator(SizedBlockAllocator* blockAllocator, void* poolStart, usz poolSizeBytes, usz blockSizeBytes);
/// Allocates a single memory block.
Result SizedBlockAllocate(SizedBlockAllocator* blockAllocator, void** block);
/// Deallocates a single memory block.
Result SizedBlockDeallocate(SizedBlockAllocator* blockAllocator, void* block);
/// Uses a `void*` to iterate on all the allocated sized blocks.
Result SizedBlockIterate(SizedBlockAllocator* blockAllocator, void** sizedBlockIterator);
/// Uses a `void*` to iterate on all the allocated sized blocks in a circle,
/// so when the end of the list is reached, it starts from the beginning.
Result SizedBlockCircularIterate(SizedBlockAllocator* blockAllocator, void** sizedBlockIterator);
/// Returns `true` for allocated blocks and `false` for the unallocated ones.
bool SizedBlockGetStatus(SizedBlockAllocator* blockAllocator, usz index);

/// Returns the memory address of the given sized block's index.
static inline void* SizedBlockGetAddress(SizedBlockAllocator* blockAllocator, usz index)
{
	return blockAllocator->FirstBlock + (blockAllocator->BlockSizeBytes * index);
}

/// Returns the index of the given sized block's memory address.
static inline usz SizedBlockGetIndex(SizedBlockAllocator* blockAllocator, void* address)
{
	return ((u8*)address - blockAllocator->FirstBlock) / blockAllocator->BlockSizeBytes;
}
