#include "Memory/FrameAllocator.hpp"

#include "Logger.hpp"
#include "Memory.hpp"
#include "Memory/Frame.hpp"

namespace SaturnKernel {

static auto NextMapEntryFrame(const MemoryMapEntry& currentEntry, const Frame<Size4KiB>& lastFrame) -> Result<Frame<Size4KiB>>
{
	const Frame<Size4KiB> minFrame(currentEntry.PhysicalStart);
	const Frame<Size4KiB> maxFrame(currentEntry.PhysicalEnd);

	// If the last allocated frame is outside the bounds of the memory descriptor,
	// we know that the first frame will be available so we return it.
	if (lastFrame < minFrame)
		return Result<Frame<Size4KiB>>::MakeOk(minFrame);

	// Check if there is one more frame to allocate, and if there is we use it.
	if (lastFrame < maxFrame)
		return Result<Frame<Size4KiB>>::MakeOk(lastFrame + 1);

	return Result<Frame<Size4KiB>>::MakeErr(ErrorCode::NotEnoughMemoryPages);
}

SequentialFrameAllocator::SequentialFrameAllocator()
	: m_lastFrame(0)
	, m_memoryMap(nullptr)
	, m_memoryMapEntries(0)
	, m_currentEntryIndex(0)
{
}

auto SequentialFrameAllocator::Init(MemoryMapEntry* memoryMap, USIZE memoryMapEntries) -> Result<void>
{
	// The memory map will only contain usable entries so we can just set the physical starting address as the last allocated frame
	if (memoryMapEntries < 1) {
		SK_LOG_ERROR("There are no usable memory map entries to allocate frames from");
		return Result<void>::MakeErr(ErrorCode::NotEnoughMemoryPages);
	}

	m_memoryMap = memoryMap;
	m_currentEntryIndex = 0;
	// Since my code will check if we exceeed the provided memory map bounds using the number of entries we can just ignore the last
	// NULL-descriptor
	// TODO: Check if its a NULL-descritptor and only then ignore it
	m_memoryMapEntries = memoryMapEntries - 1;
	m_lastFrame = Frame<Size4KiB>(memoryMap[0].PhysicalStart) - 1;

	return Result<void>::MakeOk();
}

/// A helper function that allocates the next free frame from the current descriptor,
/// only if one's available, otherwise returns an error.
auto SequentialFrameAllocator::AllocateCurrentDescriptorFrame() -> Result<Frame<Size4KiB>>
{
	auto frame = NextMapEntryFrame(m_memoryMap[m_currentEntryIndex], m_lastFrame);
	if (frame.IsError())
		return frame;

	// Because the available frame is being consumed we deem it as the previous, used one.
	m_lastFrame = frame.Value;

	return frame;
}

auto SequentialFrameAllocator::AllocateFrame() -> Result<Frame<Size4KiB>>
{
	// If there is an available frame within the current descriptor's bounds, return it.
	auto frame = AllocateCurrentDescriptorFrame();
	if (frame.IsOk())
		return frame;

	// If we are on the last descriptor we ran out of memory... Nice.
	if (m_currentEntryIndex + 1 >= m_memoryMapEntries)
		return Result<Frame<Size4KiB>>::MakeErr(ErrorCode::OutOfMemory);

	// And if not, we need to find the next available memory descriptor.
	// If its "usable", allocate the first available frame from it.
	for (USIZE i = m_currentEntryIndex + 1; i < m_memoryMapEntries; i++) {
		frame = NextMapEntryFrame(m_memoryMap[i], m_lastFrame);
		if (frame.IsOk()) {
			m_currentEntryIndex = i;
			m_lastFrame = frame.Value;

			return frame;
		}
	}

	return Result<Frame<Size4KiB>>::MakeErr(ErrorCode::OutOfMemory);
}

}
