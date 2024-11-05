#pragma once

#include "Core.hpp"

#include "Memory/Frame.hpp"
#include "Result.hpp"

namespace SaturnKernel {

struct BitmapFrameAllocator {
	BitmapFrameAllocator();

	auto Init(MemoryMapEntry* memoryMap, USIZE memoryMapEntries) -> Result<void>;

	/// Allocates a single 4 KiB memory frame.
	[[nodiscard]] auto AllocateFrame() -> Result<Frame<Size4KiB>>;
	[[nodiscard]] auto DeallocateFrame(Frame<Size4KiB> frame) -> Result<void>;

private:
	/// Marks the provided frame in the bitmap as used/unused.
	auto SetFrameStatus(Frame<Size4KiB> frame, bool used) -> void;
	/// Returns the frame status, where `true` is allocated and `false` deallocated.
	auto GetFrameStatus(Frame<Size4KiB> frame) -> bool;

	MemoryMapEntry* m_memoryMap;
	USIZE m_memoryMapEntries;
	U8* m_frameBitmap;
	Frame<Size4KiB> m_lastFrame;
};

}
