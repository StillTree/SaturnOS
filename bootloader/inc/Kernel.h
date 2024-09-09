#pragma once

#include "UefiTypes.h"
#include "FrameAllocator.h"

typedef struct KernelBootInfo
{
	EFI_VIRTUAL_ADDRESS framebufferAddress;
	UINTN               framebufferSize;
	UINTN               framebufferWidth;
	UINTN               framebufferHeight;
} KernelBootInfo;

EFI_STATUS LoadKernel(
	UINT8* loadedFile,
	FrameAllocatorData* frameAllocator,
	EFI_PHYSICAL_ADDRESS p4TableAddress,
	EFI_VIRTUAL_ADDRESS* entryPoint);

