#pragma once

#include "FrameAllocator.h"
#include "UefiTypes.h"

/// Necessary information for the kernel to properly boot.
/// It's passed in the first argument of the kernel's main function.
typedef struct KernelBootInfo {
	EFI_VIRTUAL_ADDRESS framebufferAddress;
	UINTN framebufferSize;
	UINTN framebufferWidth;
	UINTN framebufferHeight;
	EFI_VIRTUAL_ADDRESS memoryMapAddress;
	UINTN memoryMapEntries;
	UINTN physicalMemoryOffset;
	UINTN physicalMemoryMappingSize;
	EFI_PHYSICAL_ADDRESS xsdtAddress;
	EFI_VIRTUAL_ADDRESS kernelStackTop;
	UINTN kernelAddress;
	UINTN kernelSize;
} KernelBootInfo;

/// Loads the kernel ELF64 executable into memory and maps it to the provided P4 table. After this function call the kernel file's memory
/// can be deallocated.
/// Returns the entry point address in the entryPoint parameter.
///
/// Note: The whole kernel file must already be in memory before calling this function.
EFI_STATUS LoadKernel(UINT8* loadedFile, FrameAllocatorData* frameAllocator, EFI_PHYSICAL_ADDRESS p4TableAddress,
	EFI_VIRTUAL_ADDRESS* entryPoint, EFI_VIRTUAL_ADDRESS* nextUsableMemoryFrame);
