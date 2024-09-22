#pragma once

#include "UefiTypes.h"
#include "FrameAllocator.h"

// So I could make this more descriptive by adding regions marked for example
// as kernel executable, runtime services or anything else.
// But this doesn't change shit, the kernel will still boil it all down to
// usable or unusable. So I just won't bother even doing it.
typedef struct MemoryMapEntry
{
	EFI_PHYSICAL_ADDRESS physicalStart;
	UINTN                numberOfPages;
} MemoryMapEntry;

EFI_STATUS CreateMemoryMap(
	FrameAllocatorData* frameAllocator,
	EFI_PHYSICAL_ADDRESS kernelP4Table,
	MemoryMapEntry** memoryMap,
	UINTN* memoryMapEntries,
	EFI_MEMORY_DESCRIPTOR* uefiMemoryMap,
	UINTN memoryMapSize,
	UINTN descriptorSize,
	EFI_VIRTUAL_ADDRESS memoryMapVirtualAddress);

