#pragma once

#include "Core.h"
#include "Memory.h"
#include "Memory/Frame.h"
#include "Result.h"

/// A physical frame allocator based on a memory map bitmap.
typedef struct BitmapFrameAllocator {
	MemoryMapEntry* MemoryMap;
	usz MemoryMapEntries;
	u64* FrameBitmap;
	Frame4KiB LastFrame;
	Frame4KiB LastAllocated;
} BitmapFrameAllocator;

Result BitmapFrameAllocatorInit(BitmapFrameAllocator* frameAllocator, MemoryMapEntry* memoryMap, usz memoryMapEntries);

/// Allocates a single 4 KiB memory frame.
Frame4KiB AllocateFrame(BitmapFrameAllocator* frameAllocator);
/// Allocates a contiguous range of 4 KiB memory frames.
Result AllocateContiguousFrames(BitmapFrameAllocator* frameAllocator, usz count, Frame4KiB* frame);
/// Deallocates a single 4 KiB memory frame.
void DeallocateFrame(BitmapFrameAllocator* frameAllocator, Frame4KiB frame);
/// Deallocates a contiguous range of 4 KiB memory frames.
Result DeallocateContiguousFrames(BitmapFrameAllocator* frameAllocator, Frame4KiB frame, usz count);

extern BitmapFrameAllocator g_frameAllocator;
