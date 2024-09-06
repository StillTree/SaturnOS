#pragma once

#include "UefiTypes.h"
#include "FrameAllocator.h"

EFI_STATUS LoadKernel(
	UINT8* loadedFile,
	FrameAllocatorData* frameAllocator,
	EFI_PHYSICAL_ADDRESS p4TableAddress,
	EFI_VIRTUAL_ADDRESS* entryPoint);

