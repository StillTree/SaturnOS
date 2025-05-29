#pragma once

#include "Memory/SizedBlockAllocator.h"
#include "Memory/VirtualAddress.h"
#include "Result.h"

typedef struct UnusedVirtualRegion {
	/// Inclusive
	VirtualAddress Begin;
	/// Exclusive
	VirtualAddress End;
	struct UnusedVirtualRegion* Previous;
	struct UnusedVirtualRegion* Next;
} UnusedVirtualRegion;

typedef struct VirtualMemoryAllocator {
	UnusedVirtualRegion* List;
	SizedBlockAllocator ListBackingStorage;
} VirtualMemoryAllocator;

Result InitVirtualMemoryAllocator(VirtualMemoryAllocator* allocator, VirtualAddress listBeginning, usz listSize);

Result AllocateVirtualMemory(VirtualMemoryAllocator* allocator, VirtualAddress begin, VirtualAddress end);
Result DeallocateVirtualMemory(VirtualMemoryAllocator* allocator, VirtualAddress begin, VirtualAddress end);

extern VirtualMemoryAllocator g_virtualMemoryAllocator;
