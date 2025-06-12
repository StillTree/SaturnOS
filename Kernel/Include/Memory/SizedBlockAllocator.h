#pragma once

#include "Core.h"
#include "Memory/VirtualAddress.h"
#include "Result.h"

typedef struct SizedBlockAllocator {
	VirtualAddress FirstBlock;
	VirtualAddress LastBlock;
	u8* BlockBitmap;
	usz PoolSizeBytes;
	usz BlockSizeBytes;
	usz AllocationCount;
} SizedBlockAllocator;

/// Initializes the sized-block allocator. Expects a contiguous, mapped virtual memory region.
Result InitSizedBlockAllocator(SizedBlockAllocator* blockAllocator, VirtualAddress poolStart, usz poolSizeBytes, usz blockSizeBytes);
/// Allocates a single memory block.
Result AllocateSizedBlock(SizedBlockAllocator* blockAllocator, void** block);
/// Deallocates a single memory block.
Result DeallocateSizedBlock(SizedBlockAllocator* blockAllocator, void* block);
/// Returns `true` for allocated blocks and `false` for the unallocated ones.
bool GetSizedBlockStatus(SizedBlockAllocator* blockAllocator, VirtualAddress block);
