#include "Core.hpp"

#include "Result.hpp"

namespace SaturnKernel
{
	struct SequentialFrameAllocator
	{
		auto Init(MemoryMapEntry* memoryMap, USIZE memoryMapEntries) -> Result<void>;

		auto AllocateFrame() -> U64;

	private:
		auto AllocateCurrentDescriptorFrame() -> U64;

		U64 m_lastFrame;
		MemoryMapEntry* m_memoryMap;
		USIZE m_memoryMapEntries;
		USIZE m_currentEntryIndex;
	};
}
