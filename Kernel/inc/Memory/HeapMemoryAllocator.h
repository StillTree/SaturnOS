#include "Core.h"
#include "Memory/VirtualAddress.h"
#include "Result.h"

typedef struct HeapBlockHeader {
	/// Including the header's size, so in essence the usable size + 2 in bytes.
	usize Size;
	/// A pointer to the next heap block, `nullptr` if it's the last one.
	HeapBlockHeader* Next;
} HeapBlockHeader;

/// A linked list memory allocator for the kernel's needs.
typedef struct HeapMemoryAllocator {
	HeapBlockHeader Head;
} HeapMemoryAllocator;

Result HeapInit(HeapMemoryAllocator* heap, usz heapSize, VirtualAddress heapBeginning);

/// Allocates a memory block and returns a pointer to it.
Result HeapAllocate(HeapMemoryAllocator* heap, usz size, usz alignment, void** pointer);
/// Deallocates the given memory block.
Result HeapDeallocate(HeapMemoryAllocator* heap, VirtualAddress blockAddress, usz size);

/// A debugging method that I will just leave here for convenience's sake.
void HeapPrintHeaders(HeapMemoryAllocator* heap);

extern HeapMemoryAllocator g_heapMemoryAllocator;
