#include "Memory/BitmapFrameAllocator.hpp"

#include "Logger.hpp"
#include "Memory.hpp"
#include "Memory/Frame.hpp"

namespace SaturnKernel {

BitmapFrameAllocator g_frameAllocator = {};

BitmapFrameAllocator::BitmapFrameAllocator()
	: m_memoryMap(nullptr)
	, m_memoryMapEntries(0)
	, m_frameBitmap(nullptr)
	, m_lastFrame(0)
{
}

auto BitmapFrameAllocator::SetFrameStatus(Frame<Size4KiB> frame, bool used) -> void
{
	const USIZE frameIndex = frame.Address / Frame<Size4KiB>::SIZE_BYTES;
	const USIZE mapIndex = (frameIndex) / 8;
	const USIZE bitIndex = frameIndex % 8;

	if (used) {
		m_frameBitmap[mapIndex] |= (1 << bitIndex);
	} else {
		m_frameBitmap[mapIndex] &= ~(1 << bitIndex);
	}
}

auto BitmapFrameAllocator::GetFrameStatus(Frame<Size4KiB> frame) -> bool
{
	const USIZE frameIndex = frame.Address / Frame<Size4KiB>::SIZE_BYTES;
	const USIZE mapIndex = (frameIndex) / 8;
	const USIZE bitIndex = frameIndex % 8;

	return (m_frameBitmap[mapIndex] & (1 << bitIndex)) >> bitIndex == 1;
}

auto BitmapFrameAllocator::Init(MemoryMapEntry* memoryMap, USIZE memoryMapEntries) -> Result<void>
{
	// I assume the last entry is a "NULL-descriptor" so I just skip it
	const Frame<Size4KiB> lastFrame(memoryMap[memoryMapEntries - 2].PhysicalEnd + 1);

	// The number of needed frames to allocate the frame bitmap is calculated with the followinf formula:
	// number of the last frame / 8 rounded up / frame size (4096) rounded up
	const USIZE neededFrames = ((((lastFrame.Address / 4096) + 7) / 8) + Frame<Size4KiB>::SIZE_BYTES - 1) / Frame<Size4KiB>::SIZE_BYTES;

	if (neededFrames >= ((memoryMap[0].PhysicalEnd + 1 - memoryMap[0].PhysicalStart) / Frame<Size4KiB>::SIZE_BYTES)) {
		SK_LOG_ERROR("There is not enough contiguous physical frames to allocate the frame bitmap");
		return Result<void>::MakeErr(ErrorCode::NotEnoughMemoryFrames);
	}

	m_frameBitmap = reinterpret_cast<U8*>(memoryMap[0].PhysicalStart + g_bootInfo.PhysicalMemoryOffset);
	m_memoryMap = memoryMap;
	m_memoryMapEntries = memoryMapEntries;

	// Because we "allocate" the needed contiguous frames, we offset the descriptor physical start to reflect it
	memoryMap[0].PhysicalStart += neededFrames * Frame<Size4KiB>::SIZE_BYTES;
	m_lastFrame = Frame<Size4KiB>(m_memoryMap[m_memoryMapEntries - 2].PhysicalEnd);

	// We set every frame as used
	MemoryFill(m_frameBitmap, 255, neededFrames * Frame<Size4KiB>::SIZE_BYTES);

	// Then we mark frames in the memory map as unused since the map only contains available memory regions
	for (USIZE i = 0; i < memoryMapEntries - 2; i++) {
		const Frame<Size4KiB> lastFrame(m_memoryMap[i].PhysicalEnd);
		Frame<Size4KiB> frame(m_memoryMap[i].PhysicalStart);

		while (frame <= lastFrame) {
			SetFrameStatus(frame, false);
			frame++;
		}
	}

	return Result<void>::MakeOk();
}

auto BitmapFrameAllocator::AllocateFrame() -> Result<Frame<Size4KiB>>
{
	for (Frame<Size4KiB> frame(0); frame <= m_lastFrame; frame++) {
		if (GetFrameStatus(frame))
			continue;

		SetFrameStatus(frame, true);
		return Result<Frame<Size4KiB>>::MakeOk(frame);
	}

	return Result<Frame<Size4KiB>>::MakeErr(ErrorCode::OutOfMemory);
}

auto BitmapFrameAllocator::DeallocateFrame(Frame<Size4KiB> frame) -> Result<void>
{
	bool allocated = GetFrameStatus(frame);

	if (!allocated) {
		SK_LOG_WARN("An attempt was made to deallocate an unallocated memory frame");
		return Result<void>::MakeErr(ErrorCode::FrameAlreadyDeallocated);
	}

	SetFrameStatus(frame, false);

	return Result<void>::MakeOk();
}

}
