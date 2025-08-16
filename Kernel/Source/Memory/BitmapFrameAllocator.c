#include "Memory/BitmapFrameAllocator.h"

#include "Logger.h"
#include "Memory.h"
#include "Memory/Frame.h"
#include "Panic.h"

BitmapFrameAllocator g_frameAllocator = {};

static void SetFrameStatus(BitmapFrameAllocator* frameAllocator, Frame4KiB frame, bool used)
{
	const usz frameIndex = frame / FRAME_4KIB_SIZE_BYTES;
	const usz mapIndex = frameIndex / 64;
	const usz bitIndex = frameIndex % 64;

	const u64 mask = 1ULL << bitIndex;

	if (used) {
		frameAllocator->FrameBitmap[mapIndex] |= mask;
	} else {
		frameAllocator->FrameBitmap[mapIndex] &= ~mask;
	}
}

static bool GetFrameStatus(BitmapFrameAllocator* frameAllocator, Frame4KiB frame)
{
	const usz frameIndex = frame / FRAME_4KIB_SIZE_BYTES;
	const usz mapIndex = frameIndex / 64;
	const usz bitIndex = frameIndex % 64;

	const u64 mask = 1ULL << bitIndex;

	return (frameAllocator->FrameBitmap[mapIndex] & mask) != 0;
}

Result BitmapFrameAllocatorInit(BitmapFrameAllocator* frameAllocator, MemoryMapEntry* memoryMap, usz memoryMapEntries)
{
	// I assume the last entry is a "NULL-descriptor" so I just skip it
	const Frame4KiB lastFrame = Frame4KiBContaining(memoryMap[memoryMapEntries - 2].PhysicalEnd);

	// The number of needed frames to allocate the frame bitmap is calculated with the followinf formula:
	// number of the last frame / 8 rounded up / frame size rounded up (4096)
	const usz neededFrames = ((((lastFrame / FRAME_4KIB_SIZE_BYTES) + 7) / 8) + FRAME_4KIB_SIZE_BYTES - 1) / FRAME_4KIB_SIZE_BYTES;

	if (neededFrames >= ((memoryMap[0].PhysicalEnd + 1 - memoryMap[0].PhysicalStart) / FRAME_4KIB_SIZE_BYTES)) {
		LogLine(SK_LOG_ERROR "There is not enough contiguous physical frames to allocate the frame bitmap");
		return ResultNotEnoughMemoryFrames;
	}

	frameAllocator->FrameBitmap = PhysicalAddressAsPointer(memoryMap[0].PhysicalStart);
	frameAllocator->MemoryMap = memoryMap;
	frameAllocator->MemoryMapEntries = memoryMapEntries;
	frameAllocator->LastAllocated = 0;

	// Because we "allocate" the needed contiguous frames, we offset the descriptor physical start to reflect it
	memoryMap[0].PhysicalStart += neededFrames * FRAME_4KIB_SIZE_BYTES;
	frameAllocator->LastFrame = lastFrame;

	// We set every frame as used
	MemoryFill(frameAllocator->FrameBitmap, 255, neededFrames * FRAME_4KIB_SIZE_BYTES);

	// Then we mark frames in the memory map as unused since the map only contains available memory regions
	for (usz i = 0; i < memoryMapEntries - 2; i++) {
		const Frame4KiB regionEnd = Frame4KiBContaining(frameAllocator->MemoryMap[i].PhysicalEnd);
		Frame4KiB frame = Frame4KiBContaining(frameAllocator->MemoryMap[i].PhysicalStart);

		while (frame <= regionEnd) {
			SetFrameStatus(frameAllocator, frame, false);
			frame += FRAME_4KIB_SIZE_BYTES;
		}
	}

	return ResultOk;
}

Result AllocateContiguousFrames(BitmapFrameAllocator* frameAllocator, usz count, Frame4KiB* frame)
{
	for (Frame4KiB checkedFrame = frameAllocator->LastAllocated + FRAME_4KIB_SIZE_BYTES; checkedFrame <= frameAllocator->LastFrame;
		checkedFrame += FRAME_4KIB_SIZE_BYTES) {
		if (GetFrameStatus(frameAllocator, checkedFrame)) {
			continue;
		}

		bool contiguous = true;
		for (usz i = 1; i < count; i++) {
			if (GetFrameStatus(frameAllocator, checkedFrame + (i * FRAME_4KIB_SIZE_BYTES))) {
				contiguous = false;
				break;
			}
		}

		if (!contiguous) {
			checkedFrame += (count - 1) * FRAME_4KIB_SIZE_BYTES;
			continue;
		}

		for (usz i = 0; i < count; i++) {
			SetFrameStatus(frameAllocator, checkedFrame + (i * FRAME_4KIB_SIZE_BYTES), true);
		}

		*frame = checkedFrame;
		frameAllocator->LastAllocated = checkedFrame + (count - 1) * FRAME_4KIB_SIZE_BYTES;
		return ResultOk;
	}

	return ResultOutOfMemory;
}

Frame4KiB AllocateFrame(BitmapFrameAllocator* frameAllocator)
{
	for (Frame4KiB checkedFrame = frameAllocator->LastAllocated + FRAME_4KIB_SIZE_BYTES; checkedFrame <= frameAllocator->LastFrame;
		checkedFrame += FRAME_4KIB_SIZE_BYTES) {
		if (GetFrameStatus(frameAllocator, checkedFrame)) {
			continue;
		}

		SetFrameStatus(frameAllocator, checkedFrame, true);
		frameAllocator->LastAllocated = checkedFrame;

		return checkedFrame;
	}

	SK_PANIC("The kernel ran out of memory");
}

Result DeallocateContiguousFrames(BitmapFrameAllocator* frameAllocator, Frame4KiB frame, usz count)
{
	if (frame + (count * FRAME_4KIB_SIZE_BYTES) > frameAllocator->LastFrame) {
		return ResultOutOfRange;
	}

	for (usz i = 0; i < count; i++) {
		Frame4KiB currentFrame = frame + (i * FRAME_4KIB_SIZE_BYTES);
		if (!GetFrameStatus(frameAllocator, currentFrame)) {
			return ResultFrameAlreadyDeallocated;
		}
	}

	for (usz i = 0; i < count; i++) {
		SetFrameStatus(frameAllocator, frame + (i * FRAME_4KIB_SIZE_BYTES), false);
	}

	frameAllocator->LastAllocated = 0;

	return ResultOk;
}

void DeallocateFrame(BitmapFrameAllocator* frameAllocator, Frame4KiB frame)
{
	bool allocated = GetFrameStatus(frameAllocator, frame);

	if (!allocated) {
		LogLine(SK_LOG_WARN "An attempt was made to deallocate an unallocated memory frame");
	}

	SetFrameStatus(frameAllocator, frame, false);
	frameAllocator->LastAllocated = 0;
}
