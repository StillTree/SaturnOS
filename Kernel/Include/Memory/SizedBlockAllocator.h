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
} SizedBlockAllocator;

/// Expects a contiguous virtual memory region.
Result InitSizedBlockAllocator(SizedBlockAllocator* blockAllocator, VirtualAddress poolStart, usz poolSizeBytes, usz blockSizeBytes);
Result AllocateSizedBlock(SizedBlockAllocator* blockAllocator, void** block);
Result DeallocateSizedBlock(SizedBlockAllocator* blockAllocator, void* block);
