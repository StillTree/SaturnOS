#pragma once

#include "FrameAllocator.h"
#include "UefiTypes.h"

// So I could make this more descriptive by adding regions marked for example
// as kernel executable, runtime services or anything else.
// But this doesn't change shit, the kernel will still boil it all down to
// usable or unusable. So I just won't bother even doing it.
/// Marks a usable memory map entry for the kernel.
typedef struct MemoryMapEntry {
	EFI_PHYSICAL_ADDRESS physicalStart;
	UINTN physicalEnd;
} MemoryMapEntry;

/// Creates and maps for the kernel a memory map based on the known UEFI memory map and already allocated frames
/// by the sequential frame allocator used. Outputs the number of entries in the kernel memory map.
EFI_STATUS CreateMemoryMap(FrameAllocatorData* frameAllocator, EFI_PHYSICAL_ADDRESS kernelP4Table, UINTN* memoryMapEntries,
	EFI_MEMORY_DESCRIPTOR* uefiMemoryMap, UINTN memoryMapSize, UINTN descriptorSize, EFI_VIRTUAL_ADDRESS memoryMapVirtualAddress);
