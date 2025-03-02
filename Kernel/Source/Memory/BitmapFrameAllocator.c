#include "Memory/BitmapFrameAllocator.h"

#include "Logger.h"
#include "Memory.h"
#include "Memory/Frame.h"

BitmapFrameAllocator g_frameAllocator = {};

static void SetFrameStatus(BitmapFrameAllocator* frameAllocator, Frame4KiB frame, bool used)
{
	const usz frameIndex = frame / FRAME_4KIB_SIZE_BYTES;
	const usz mapIndex = (frameIndex) / 8;
	const usz bitIndex = frameIndex % 8;

	if (used) {
		frameAllocator->FrameBitmap[mapIndex] |= (1 << bitIndex);
	} else {
		frameAllocator->FrameBitmap[mapIndex] &= ~(1 << bitIndex);
	}
}

static bool GetFrameStatus(BitmapFrameAllocator* frameAllocator, Frame4KiB frame)
{
	const usz frameIndex = frame / FRAME_4KIB_SIZE_BYTES;
	const usz mapIndex = (frameIndex) / 8;
	const usz bitIndex = frameIndex % 8;

	return ((frameAllocator->FrameBitmap[mapIndex] & (1 << bitIndex)) >> bitIndex) == 1;
}

Result BitmapFrameAllocatorInit(BitmapFrameAllocator* frameAllocator, MemoryMapEntry* memoryMap, usz memoryMapEntries)
{
	// I assume the last entry is a "NULL-descriptor" so I just skip it
	const Frame4KiB lastFrame = Frame4KiBContainingAddress(memoryMap[memoryMapEntries - 2].PhysicalEnd + 1);

	// The number of needed frames to allocate the frame bitmap is calculated with the followinf formula:
	// number of the last frame / 8 rounded up / frame size (4096) rounded up
	const usz neededFrames = ((((lastFrame / 4096) + 7) / 8) + FRAME_4KIB_SIZE_BYTES - 1) / FRAME_4KIB_SIZE_BYTES;

	if (neededFrames >= ((memoryMap[0].PhysicalEnd + 1 - memoryMap[0].PhysicalStart) / FRAME_4KIB_SIZE_BYTES)) {
		SK_LOG_ERROR("There is not enough contiguous physical frames to allocate the frame bitmap");
		return ResultNotEnoughMemoryFrames;
	}

	frameAllocator->FrameBitmap = (u8*)PhysicalAddressAsPointer(memoryMap[0].PhysicalStart);
	frameAllocator->MemoryMap = memoryMap;
	frameAllocator->MemoryMapEntries = memoryMapEntries;

	// Because we "allocate" the needed contiguous frames, we offset the descriptor physical start to reflect it
	memoryMap[0].PhysicalStart += neededFrames * FRAME_4KIB_SIZE_BYTES;
	frameAllocator->LastFrame = Frame4KiBContainingAddress(frameAllocator->MemoryMap[frameAllocator->MemoryMapEntries - 2].PhysicalEnd);

	// We set every frame as used
	MemoryFill(frameAllocator->FrameBitmap, 255, neededFrames * FRAME_4KIB_SIZE_BYTES);

	// Then we mark frames in the memory map as unused since the map only contains available memory regions
	for (usz i = 0; i < memoryMapEntries - 2; i++) {
		const Frame4KiB lastFrame = Frame4KiBContainingAddress(frameAllocator->MemoryMap[i].PhysicalEnd);
		Frame4KiB frame = Frame4KiBContainingAddress(frameAllocator->MemoryMap[i].PhysicalStart);

		while (frame <= lastFrame) {
			SetFrameStatus(frameAllocator, frame, false);
			frame += FRAME_4KIB_SIZE_BYTES;
		}
	}

	return ResultOk;
}

Result AllocateContiguousFrames(BitmapFrameAllocator* frameAllocator, usz number, Frame4KiB* frame)
{
	for (Frame4KiB checkedFrame = 0; checkedFrame <= frameAllocator->LastFrame; checkedFrame += FRAME_4KIB_SIZE_BYTES) {
		if (GetFrameStatus(frameAllocator, checkedFrame))
			continue;

		bool contiguous = true;
		for (usz i = 1; i < number; i++) {
			if (GetFrameStatus(frameAllocator, checkedFrame + (i * FRAME_4KIB_SIZE_BYTES))) {
				contiguous = false;
				break;
			}
		}

		if (!contiguous) {
			checkedFrame += (number - 1) * FRAME_4KIB_SIZE_BYTES;
			continue;
		}

		for (usz i = 0; i < number; i++) {
			SetFrameStatus(frameAllocator, checkedFrame + (i * FRAME_4KIB_SIZE_BYTES), true);
		}

		*frame = checkedFrame;
		return ResultOk;
	}

	return ResultOutOfMemory;
}

Result AllocateFrame(BitmapFrameAllocator* frameAllocator, Frame4KiB* frame)
{
	for (Frame4KiB checkedFrame = 0; checkedFrame <= frameAllocator->LastFrame; checkedFrame += FRAME_4KIB_SIZE_BYTES) {
		if (GetFrameStatus(frameAllocator, checkedFrame)) {
			continue;
		}

		SetFrameStatus(frameAllocator, checkedFrame, true);
		*frame = checkedFrame;
		return ResultOk;
	}

	return ResultOutOfMemory;
}

Result DeallocateContiguousFrames(BitmapFrameAllocator* frameAllocator, Frame4KiB frame, usz number)
{
	if (frame + (number * FRAME_4KIB_SIZE_BYTES) > frameAllocator->LastFrame) {
		return ResultOutOfRange;
	}

	for (usz i = 0; i < number; i++) {
		Frame4KiB currentFrame = frame + (i * FRAME_4KIB_SIZE_BYTES);
		if (!GetFrameStatus(frameAllocator, currentFrame)) {
			return ResultFrameAlreadyDeallocated;
		}
	}

	for (usz i = 0; i < number; i++) {
		SetFrameStatus(frameAllocator, frame + (i * FRAME_4KIB_SIZE_BYTES), false);
	}

	return ResultOk;
}

void DeallocateFrame(BitmapFrameAllocator* frameAllocator, Frame4KiB frame)
{
	bool allocated = GetFrameStatus(frameAllocator, frame);

	if (!allocated) {
		SK_LOG_WARN("An attempt was made to deallocate an unallocated memory frame");
	}

	SetFrameStatus(frameAllocator, frame, false);
}
