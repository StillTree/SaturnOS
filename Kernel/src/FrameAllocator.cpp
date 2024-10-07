#include "FrameAllocator.hpp"

#include "Logger.hpp"

namespace SaturnKernel
{
	void SequentialFrameAllocator::Init(MemoryMapEntry* memoryMap, USIZE memoryMapEntries)
	{
		// The memory map will only contain usable entries so we can just set the physical starting address as the last allocated frame and
		// don't give a shit if the memory map has any entries at all 🗿

		if(m_memoryMapEntries < 1)
		{
			SK_LOG_ERROR("There are no usable memory map entries to allocate frames from");
			return;
		}

		m_memoryMap			= memoryMap;
		m_currentEntryIndex = 0;
		m_memoryMapEntries	= memoryMapEntries;
		m_lastFrame			= memoryMap[0].physicalStart - 4096;
	}

	U64 SequentialFrameAllocator::AllocateFrame()
	{
	}
}
