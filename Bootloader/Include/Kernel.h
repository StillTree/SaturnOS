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
	EFI_VIRTUAL_ADDRESS contextSwitchFunctionPage;
	EFI_VIRTUAL_ADDRESS ramdiskAddress;
	UINTN ramdiskSizeBytes;
	EFI_VIRTUAL_ADDRESS kernelArgsAddress;
	EFI_PHYSICAL_ADDRESS kernelP4TableAddress;
} KernelBootInfo;

/// Loads the kernel arguments from the bootloader's config file (`KernelArgs`),
/// allocates a frame for them and maps it in the kernel's memory space.
///
/// Note: The whole bootloader's config file must already be in memory before calling this function.
EFI_STATUS LoadKernelArgs(const INT8* configFile, UINTN configFileSize, FrameAllocatorData* frameAllocator,
	EFI_PHYSICAL_ADDRESS p4TableAddress, EFI_VIRTUAL_ADDRESS* nextUsableVirtualPage);

/// Loads the kernel ELF64 executable into memory and maps it to the provided P4 table. After this function call the kernel file's memory
/// can be deallocated.
/// Returns the entry point address in the entryPoint parameter.
///
/// Note: The whole kernel file must already be in memory before calling this function.
EFI_STATUS LoadKernel(UINT8* loadedFile, FrameAllocatorData* frameAllocator, EFI_PHYSICAL_ADDRESS p4TableAddress,
	EFI_VIRTUAL_ADDRESS* entryPoint, EFI_VIRTUAL_ADDRESS* nextUsableVirtualPage);
