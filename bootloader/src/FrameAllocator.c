#include "FrameAllocator.h"

#include "Logger.h"

/// Returns the starting address for a physical frame that contains the given address
/// (literally just aligns the address to the lower 4096 byte).
EFI_PHYSICAL_ADDRESS PhysFrameContainingAddress(EFI_PHYSICAL_ADDRESS address)
{
	// As simple as it gets...
	return address & ~0xFFF;
}

/// Returns the next available frame from the given descriptor starting with `lastFrame` (exclusive).
EFI_STATUS NextDescriptorFrame(
	EFI_MEMORY_DESCRIPTOR* memoryDescriptor,
	EFI_PHYSICAL_ADDRESS lastFrame,
	EFI_PHYSICAL_ADDRESS* allocatedFrameAddress)
{
	const EFI_PHYSICAL_ADDRESS minFrame = PhysFrameContainingAddress(memoryDescriptor->PhysicalStart);
	const EFI_PHYSICAL_ADDRESS maxFrame = PhysFrameContainingAddress(memoryDescriptor->PhysicalStart + memoryDescriptor->NumberOfPages * 4096 - 1);

	// If the last allocated frame is outside the bounds of the memory descriptor,
	// we know that the first frame will be available so we return it.
	if(lastFrame < minFrame)
	{
		*allocatedFrameAddress = minFrame;
		return EFI_SUCCESS;
	}

	// Check if there is one more frame to allocate, and if there is we use it.
	if(lastFrame < maxFrame)
	{
		*allocatedFrameAddress = lastFrame + 4096;
		return EFI_SUCCESS;
	}

	return EFI_OUT_OF_RESOURCES;
}

EFI_STATUS InitFrameAllocator(FrameAllocatorData* frameAllocator, EFI_MEMORY_DESCRIPTOR* memoryMap, UINTN memoryMapSize, UINTN descriptorSize)
{
	frameAllocator->currentMemoryDescriptor      = memoryMap;
	frameAllocator->memoryMapSize                = memoryMapSize;
	frameAllocator->descriptorSize               = descriptorSize;
	frameAllocator->currentMemoryDescriptorIndex = 0;

	BOOLEAN found = FALSE;
	// Find the first usable descriptor
	for(UINTN i = 0; i < memoryMapSize / descriptorSize; i++)
	{
		// sizeof(EFI_MEMORY_DESCRIPTOR) is not the same as its size in memory
		EFI_MEMORY_DESCRIPTOR* descriptor = (EFI_MEMORY_DESCRIPTOR*) ((UINT8*) memoryMap + (i * descriptorSize));

		if(descriptor->Type == EfiConventionalMemory)
		{
			frameAllocator->currentMemoryDescriptor      = descriptor;
			frameAllocator->currentMemoryDescriptorIndex = i;
			found                                        = TRUE;
			break;
		}
	}

	// Theoretically there is no usable memory so we return an error
	if(!found)
	{
		SN_LOG_ERROR(L"Could not find a usable memory descriptor");
		return EFI_NOT_FOUND;
	}

	frameAllocator->previousFrame = PhysFrameContainingAddress(MIN_PHYS_MEMORY_ADDRESS);

	return EFI_SUCCESS;
}

/// A helper function that allocates the next free frame from the current descriptor,
/// only if one's available, otherwise returns an error.
EFI_STATUS AllocateNextCurrentDescriptorFrame(FrameAllocatorData* frameAllocator, EFI_PHYSICAL_ADDRESS* allocatedFrameAddress)
{
	EFI_STATUS status = NextDescriptorFrame(
		frameAllocator->currentMemoryDescriptor,
		frameAllocator->previousFrame,
		allocatedFrameAddress);
	if(EFI_ERROR(status))
	{
		return status;
	}

	// Because the available frame is being consumed we deem it as the previous, used one.
	frameAllocator->previousFrame = *allocatedFrameAddress;

	return status;
}

EFI_STATUS AllocateFrame(FrameAllocatorData* frameAllocator, EFI_PHYSICAL_ADDRESS* allocatedFrameAddress)
{
	// If there is an available frame within the current descriptor's bounds, return it.
	EFI_STATUS status = AllocateNextCurrentDescriptorFrame(frameAllocator, allocatedFrameAddress);
	if(!EFI_ERROR(status))
	{
		return EFI_SUCCESS;
	}

	// And if not, we need to find the next available memory descriptor.
	// If its "usable", allocate the first available frame from it.
	for(UINTN i = 1; i < (frameAllocator->memoryMapSize / frameAllocator->descriptorSize) - frameAllocator->currentMemoryDescriptorIndex; i++)
	{
		// sizeof(EFI_MEMORY_DESCRIPTOR) is not the same as its size in memory
		EFI_MEMORY_DESCRIPTOR* descriptor =
			(EFI_MEMORY_DESCRIPTOR*) ((UINT8*) frameAllocator->currentMemoryDescriptor + (i * frameAllocator->descriptorSize));

		if(descriptor->Type != EfiConventionalMemory)
		{
			continue;
		}

		status = NextDescriptorFrame(
			descriptor,
			frameAllocator->previousFrame,
			allocatedFrameAddress);
		if(!EFI_ERROR(status))
		{
			frameAllocator->currentMemoryDescriptor      = descriptor;
			frameAllocator->currentMemoryDescriptorIndex += i;
			frameAllocator->previousFrame                = *allocatedFrameAddress;
			return EFI_SUCCESS;
		}
	}

	return EFI_OUT_OF_RESOURCES;
}

