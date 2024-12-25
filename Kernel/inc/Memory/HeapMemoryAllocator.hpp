#include "Core.hpp"
#include "Memory/VirtualAddress.hpp"
#include "Result.hpp"

namespace SaturnKernel {

struct HeapBlockHeader {
	/// Including the header's size, so in essence the usable size + 2 in bytes.
	USIZE Size;
	/// A pointer to the next heap block, `nullptr` if it's the last one.
	HeapBlockHeader* Next;
};

/// A linked list memory allocator for the kernel's needs.
struct HeapMemoryAllocator {
	HeapMemoryAllocator();

	auto Init(USIZE heapSize, VirtualAddress heapBeginning) -> Result<void>;

	/// Allocates a memory block and returns a pointer to it.
	auto Allocate(USIZE size, USIZE alignment) -> Result<void*>;
	/// Deallocates the given memory block.
	auto Deallocate(VirtualAddress blockAddress, USIZE size) -> Result<void>;

	/// A debugging method that I will just leave here for convenience's sake.
	auto PrintHeaders() -> void;

private:
	/// Places a memory region with the provided size at the given memory address and adds it to the list.
	///
	/// Note: This method just creates the region as is, without any checks!
	auto AddFreeRegion(VirtualAddress address, USIZE size) -> Result<void>;
	/// Resizes the given block and if able, creates a new one with the remaining space.
	auto SplitBlock(HeapBlockHeader* block, USIZE newSize) -> Result<void>;

	HeapBlockHeader m_head;
};

extern HeapMemoryAllocator g_heapMemoryAllocator;

}
