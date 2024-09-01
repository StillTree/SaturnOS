#pragma once

#include "Uefi.h"

/// Frame allocator's state.
typedef struct FrameAllocatorData
{
	UINTN memoryMapSize;
	UINTN descriptorSize;
	EFI_MEMORY_DESCRIPTOR* memoryMap;
} FrameAllocatorData;

