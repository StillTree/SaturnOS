#pragma once

#include "Core.hpp"

#include "Memory/Frame.hpp"
#include "Result.hpp"

namespace SaturnKernel {

struct SequentialFrameAllocator {
	SequentialFrameAllocator();

	auto Init(MemoryMapEntry* memoryMap, USIZE memoryMapEntries) -> Result<void>;

	auto AllocateFrame() -> Result<Frame<Size4KiB>>;

private:
	auto AllocateCurrentDescriptorFrame() -> Result<Frame<Size4KiB>>;

	Frame<Size4KiB> m_lastFrame;
	MemoryMapEntry* m_memoryMap;
	USIZE m_memoryMapEntries;
	USIZE m_currentEntryIndex;
};

}
