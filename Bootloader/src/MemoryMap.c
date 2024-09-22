#include "MemoryMap.h"

#include "FrameAllocator.h"
#include "Memory.h"

EFI_STATUS CreateMemoryMap(
	FrameAllocatorData* frameAllocator,
	EFI_PHYSICAL_ADDRESS kernelP4Table,
	MemoryMapEntry** memoryMap,
	EFI_MEMORY_DESCRIPTOR* uefiMemoryMap,
	UINTN memoryMapSize,
	UINTN descriptorSize,
	EFI_VIRTUAL_ADDRESS memoryMapVirtualAddress)
{
	EFI_STATUS status = EFI_SUCCESS;

	UINTN entries = 0;

	// Figure out the number of needed entries, we can throw out the unused ones
	for(UINTN i = 0; i < memoryMapSize / descriptorSize; i++)
	{
		// sizeof(EFI_MEMORY_DESCRIPTOR) is not the same as its size in memory
		EFI_MEMORY_DESCRIPTOR* descriptor = (EFI_MEMORY_DESCRIPTOR*) ((UINT8*) uefiMemoryMap + (i * descriptorSize));

		if(descriptor->Type == EfiConventionalMemory ||
			descriptor->Type == EfiLoaderData || 
			descriptor->Type == EfiLoaderCode ||
			descriptor->Type == EfiBootServicesData ||
			descriptor->Type == EfiBootServicesCode)
		{
			entries++;
		}
	}

	// Allocate them and map to the given virtual address
	UINTN memoryMapPages = (entries * sizeof(MemoryMapEntry) + 4095) / 4096;
	EFI_PHYSICAL_ADDRESS mapPhysicalAddress;
	status = AllocateContiguousFrames(frameAllocator, memoryMapPages, &mapPhysicalAddress);
	if(EFI_ERROR(status))
	{
		return status;
	}

	for(UINTN i = 0; i < memoryMapPages; i++)
	{
		status = MapMemoryPage(
			memoryMapVirtualAddress,
			mapPhysicalAddress + i * 4096,
			kernelP4Table,
			frameAllocator,
			ENTRY_PRESENT | ENTRY_WRITEABLE | ENTRY_NO_EXECUTE);
		if(EFI_ERROR(status))
		{
			return status;
		}

		memoryMapVirtualAddress += 4096;
	}

	MemoryMapEntry* firstEntry = (MemoryMapEntry*) mapPhysicalAddress;

	return status;
}

