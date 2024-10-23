#include "FrameAllocator.hpp"

#include "Logger.hpp"
#include "Memory.hpp"

namespace SaturnKernel
{
	static auto NextMapEntryFrame(const MemoryMapEntry& currentEntry, U64 lastFrame) -> U64
	{
		const U64 minFrame = PhysFrameContainingAddress(currentEntry.PhysicalStart);
		const U64 maxFrame = PhysFrameContainingAddress(currentEntry.PhysicalEnd);

		// If the last allocated frame is outside the bounds of the memory descriptor,
		// we know that the first frame will be available so we return it.
		if(lastFrame < minFrame)
		{
			return minFrame;
		}

		// Check if there is one more frame to allocate, and if there is we use it.
		if(lastFrame < maxFrame)
		{
			return lastFrame + 4096;
		}

		return 0xffffffffffffffff;
	}

	auto SequentialFrameAllocator::Init(MemoryMapEntry* memoryMap, USIZE memoryMapEntries) -> Result<void>
	{
		// The memory map will only contain usable entries so we can just set the physical starting address as the last allocated frame
		if(memoryMapEntries < 1)
		{
			SK_LOG_ERROR("There are no usable memory map entries to allocate frames from");
			return Result<void>::MakeErr(ErrorCode::NotEnoughMemoryPages);
		}

		m_memoryMap			= memoryMap;
		m_currentEntryIndex = 0;
		// Since my code will check if we exceeed the provided memory map bounds using the number of entries we can just ignore the last
		// NULL-descriptor
		// TODO: Check if its a NULL-descritptor and only then ignore it
		m_memoryMapEntries = memoryMapEntries - 1;
		m_lastFrame		   = memoryMap[0].PhysicalStart - 4096;

		return Result<void>::MakeOk();
	}

	/// A helper function that allocates the next free frame from the current descriptor,
	/// only if one's available, otherwise returns an error.
	auto SequentialFrameAllocator::AllocateCurrentDescriptorFrame() -> U64
	{
		U64 frame = NextMapEntryFrame(m_memoryMap[m_currentEntryIndex], m_lastFrame);
		if(frame == 0xffffffffffffffff)
			return frame;

		// Because the available frame is being consumed we deem it as the previous, used one.
		m_lastFrame = frame;

		return frame;
	}

	auto SequentialFrameAllocator::AllocateFrame() -> U64
	{
		// If there is an available frame within the current descriptor's bounds, return it.
		U64 frame = AllocateCurrentDescriptorFrame();
		if(frame != 0xffffffffffffffff)
			return frame;

		// If we are on the last descriptor we ran out of memory... Nice.
		if(m_currentEntryIndex + 1 >= m_memoryMapEntries)
			return 0xffffffffffffffff;

		// And if not, we need to find the next available memory descriptor.
		// If its "usable", allocate the first available frame from it.
		for(USIZE i = m_currentEntryIndex + 1; i < m_memoryMapEntries; i++)
		{
			frame = NextMapEntryFrame(m_memoryMap[i], m_lastFrame);
			if(frame != 0xffffffffffffffff)
			{
				m_currentEntryIndex = i;
				m_lastFrame			= frame;

				return frame;
			}
		}

		return 0xffffffffffffffff;
	}
}
