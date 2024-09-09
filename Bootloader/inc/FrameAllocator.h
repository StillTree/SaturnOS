#pragma once

#include "Uefi.h"

// We don't talk about what's below it...
#define MIN_PHYS_MEMORY_ADDRESS (EFI_PHYSICAL_ADDRESS) 10000

/// Frame allocator's state.
typedef struct FrameAllocatorData
{
	UINTN                  memoryMapSize;
	UINTN                  descriptorSize;
	EFI_MEMORY_DESCRIPTOR* currentMemoryDescriptor;
	UINTN                  currentMemoryDescriptorIndex; // Relative to the whole memory map
	EFI_PHYSICAL_ADDRESS   previousFrame;
} FrameAllocatorData;

/// Creates a new sequential physical frame allocator based on the provided memory map.
EFI_STATUS InitFrameAllocator(FrameAllocatorData* frameAllocator, EFI_MEMORY_DESCRIPTOR* memoryMap, UINTN memoryMapSize, UINTN descriptorSize);
/// Allocates a new available memory frame.
EFI_STATUS AllocateFrame(FrameAllocatorData* frameAllocator, EFI_PHYSICAL_ADDRESS* allocatedFrameAddress);

