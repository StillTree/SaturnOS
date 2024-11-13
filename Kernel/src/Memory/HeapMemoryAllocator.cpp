#include "Memory/HeapMemoryAllocator.hpp"

#include "Memory/Page.hpp"

namespace SaturnKernel {

HeapMemoryAllocator::HeapMemoryAllocator()
	: m_head(nullptr)
{
}

auto HeapMemoryAllocator::Init(USIZE heapSize, VirtualAddress heapBeginning) -> Result<void>
{
	Page<Size4KiB> heapPage(heapBeginning);

	return Result<void>::MakeOk();
}

auto HeapMemoryAllocator::Allocate(USIZE blockSize) -> Result<VirtualAddress>
{
}

auto HeapMemoryAllocator::Deallocate(VirtualAddress blockAddress) -> Result<void>
{
}

}

