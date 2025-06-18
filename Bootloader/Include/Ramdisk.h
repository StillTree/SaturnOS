#pragma once

#include "FrameAllocator.h"
#include "UefiTypes.h"

EFI_STATUS LoadRamdisk(UINT8* loadedFile, UINTN fileSize, FrameAllocatorData* frameAllocator, EFI_PHYSICAL_ADDRESS p4TableAddress,
	EFI_VIRTUAL_ADDRESS* nextUsableVirtualPage);
