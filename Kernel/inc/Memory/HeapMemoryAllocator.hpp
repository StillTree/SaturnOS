#include "Core.hpp"
#include "Memory/VirtualAddress.hpp"
#include "Result.hpp"

namespace SaturnKernel {

struct HeapBlockHeader {
	USIZE BlockSize;
	HeapBlockHeader* NextBlock;
};

struct HeapMemoryAllocator {
	HeapMemoryAllocator();

	auto Init(USIZE heapSize, VirtualAddress heapBeginning) -> Result<void>;

	auto Allocate(USIZE blockSize) -> Result<VirtualAddress>;
	auto Deallocate(VirtualAddress blockAddress) -> Result<void>;

private:
	HeapBlockHeader* m_head;
};

}
