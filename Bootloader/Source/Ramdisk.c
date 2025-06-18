#include "Ramdisk.h"

#include "Logger.h"
#include "Memory.h"

EFI_STATUS LoadRamdisk(UINT8* loadedFile, UINTN fileSize, FrameAllocatorData* frameAllocator, EFI_PHYSICAL_ADDRESS p4TableAddress,
	EFI_VIRTUAL_ADDRESS* nextUsableVirtualPage)
{
	EFI_STATUS status = EFI_SUCCESS;
	UINTN ramdiskFrames = (fileSize + 4095) / 4096;

	for (UINTN i = 0; i < ramdiskFrames; i++) {
		EFI_PHYSICAL_ADDRESS frameAddress = 0;
		status = AllocateFrame(frameAllocator, &frameAddress);
		if (EFI_ERROR(status)) {
			SN_LOG_ERROR(L"An unexpected error occured while trying to allocate a memory frame for the ramdisk");
			return status;
		}

		UINTN copySize = fileSize - (i * 4096);
		if (copySize > 4096) {
			copySize = 4096;
		}

		MemoryCopy(loadedFile + (i * 4096), (VOID*)frameAddress, copySize);

		status = MapMemoryPage4KiB(*nextUsableVirtualPage, frameAddress, p4TableAddress, frameAllocator, ENTRY_PRESENT | ENTRY_NO_EXECUTE);
		if (EFI_ERROR(status)) {
			SN_LOG_ERROR(L"An unexpected error occured while trying to map a memory frame in the kernel's P4 table");
			return status;
		}

		*nextUsableVirtualPage += 4096;
	}

	return status;
}
