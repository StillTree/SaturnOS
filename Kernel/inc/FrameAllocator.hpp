#include "Core.hpp"

namespace SaturnKernel
{
	struct SequentialFrameAllocator
	{
		void Init(MemoryMapEntry* memoryMap, USIZE memoryMapEntries);

		auto AllocateFrame() -> U64;

	private:
		auto AllocateCurrentDescriptorFrame() -> U64;

		U64 m_lastFrame;
		MemoryMapEntry* m_memoryMap;
		USIZE m_memoryMapEntries;
		USIZE m_currentEntryIndex;
	};
}
