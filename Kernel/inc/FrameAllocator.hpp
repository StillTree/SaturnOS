#include "Core.hpp"

namespace SaturnKernel
{
	struct SequentialFrameAllocator
	{
		void Init(MemoryMapEntry* memoryMap, USIZE memoryMapEntries);

		U64 AllocateFrame();

	private:
		U64 m_lastFrame;
		MemoryMapEntry* m_memoryMap;
		USIZE m_memoryMapEntries;
		USIZE m_currentEntryIndex;
	};
}
