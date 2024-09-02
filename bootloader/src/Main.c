#include "Uefi.h"
#include "Logger.h"
#include "FrameAllocator.h"

EFI_GUID gEfiGraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
EFI_GUID gEfiSimpleTextOutProtocolGuid  = EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID;

EFI_STATUS ExitBootServices(
	EFI_HANDLE* imageHandle,
	EFI_SYSTEM_TABLE* systemTable,
	EFI_MEMORY_DESCRIPTOR** memoryMap,
	UINTN* descriptorSize,
	UINTN* memoryMapSize);

EFI_STATUS EFIAPI UefiMain(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable)
{
	(void) imageHandle;
	EFI_STATUS status = InitLogger(systemTable, &g_mainLogger, TRUE, TRUE);
	if(EFI_ERROR(status))
	{
		goto halt;
	}

	UINTN descriptorSize = 0;
	UINTN memoryMapSize = 0;
	EFI_MEMORY_DESCRIPTOR* memoryMap = NULL;
	status = ExitBootServices(imageHandle, systemTable, &memoryMap, &descriptorSize, &memoryMapSize);
	if(EFI_ERROR(status))
	{
		goto halt;
	}

	SN_LOG_INFO(L"Successfully exited boot services");

	FrameAllocatorData frameAllocator = { 0, 0, NULL, 0, 0 };
	status = InitFrameAllocator(&frameAllocator, memoryMap, memoryMapSize, descriptorSize);
	if(EFI_ERROR(status))
	{
		goto halt;
	}

	EFI_PHYSICAL_ADDRESS address = 0;
	status = AllocateFrame(&frameAllocator, &address);
	if(EFI_ERROR(status))
	{
		SN_LOG_INFO(L"allocate 19");
		goto halt;
	}
	status = AllocateFrame(&frameAllocator, &address);
	if(EFI_ERROR(status))
	{
		SN_LOG_INFO(L"allocate 18");
		goto halt;
	}
	status = AllocateFrame(&frameAllocator, &address);
	if(EFI_ERROR(status))
	{
		SN_LOG_INFO(L"allocate 17");
		goto halt;
	}
	status = AllocateFrame(&frameAllocator, &address);
	if(EFI_ERROR(status))
	{
		SN_LOG_INFO(L"allocate 16");
		goto halt;
	}
	status = AllocateFrame(&frameAllocator, &address);
	if(EFI_ERROR(status))
	{
		SN_LOG_INFO(L"allocate 15");
		goto halt;
	}
	status = AllocateFrame(&frameAllocator, &address);
	if(EFI_ERROR(status))
	{
		SN_LOG_INFO(L"allocate 14");
		goto halt;
	}
	status = AllocateFrame(&frameAllocator, &address);
	if(EFI_ERROR(status))
	{
		SN_LOG_INFO(L"allocate 13");
		goto halt;
	}
	status = AllocateFrame(&frameAllocator, &address);
	if(EFI_ERROR(status))
	{
		SN_LOG_INFO(L"allocate 12");
		goto halt;
	}
	status = AllocateFrame(&frameAllocator, &address);
	if(EFI_ERROR(status))
	{
		SN_LOG_INFO(L"allocate 11");
		goto halt;
	}
	status = AllocateFrame(&frameAllocator, &address);
	if(EFI_ERROR(status))
	{
		SN_LOG_INFO(L"allocate 10");
		goto halt;
	}
	status = AllocateFrame(&frameAllocator, &address);
	if(EFI_ERROR(status))
	{
		SN_LOG_INFO(L"allocate 9");
		goto halt;
	}
	status = AllocateFrame(&frameAllocator, &address);
	if(EFI_ERROR(status))
	{
		SN_LOG_INFO(L"allocate 8");
		goto halt;
	}
	status = AllocateFrame(&frameAllocator, &address);
	if(EFI_ERROR(status))
	{
		SN_LOG_INFO(L"allocate 7");
		goto halt;
	}
	status = AllocateFrame(&frameAllocator, &address);
	if(EFI_ERROR(status))
	{
		SN_LOG_INFO(L"allocate 6");
		goto halt;
	}
	status = AllocateFrame(&frameAllocator, &address);
	if(EFI_ERROR(status))
	{
		SN_LOG_INFO(L"allocate 5");
		goto halt;
	}
	status = AllocateFrame(&frameAllocator, &address);
	if(EFI_ERROR(status))
	{
		SN_LOG_INFO(L"allocate 4");
		goto halt;
	}
	status = AllocateFrame(&frameAllocator, &address);
	if(EFI_ERROR(status))
	{
		SN_LOG_INFO(L"allocate 3");
		goto halt;
	}
	status = AllocateFrame(&frameAllocator, &address);
	if(EFI_ERROR(status))
	{
		SN_LOG_INFO(L"allocate 2");
		goto halt;
	}
	status = AllocateFrame(&frameAllocator, &address);
	if(EFI_ERROR(status))
	{
		SN_LOG_INFO(L"allocate 1");
		goto halt;
	}
	status = AllocateFrame(&frameAllocator, &address);
	if(EFI_ERROR(status))
	{
		SN_LOG_INFO(L"allocate 0");
		goto halt;
	}

	SN_LOG_INFO(L"yoo...");

halt:
	while(TRUE)
	{
		__asm__("cli; hlt");
	}

	// We never ever want to get here
	return EFI_SUCCESS;
}

UINTN uint64_to_str_length(UINTN value) {
    UINTN length = 0;
    
    // Handle zero case explicitly, as "0" has a length of 1
    if (value == 0) {
        return 2; // 1 for '0' and 1 for '\0'
    }

    // Calculate the number of digits in the number
    while (value > 0) {
        value /= 10;
        length++;
    }

    // Add 1 for the null terminator
    return length + 1;
}

void uint64_to_str(UINTN value, char *buffer) {
    // Calculate the length needed for the string
    UINTN length = uint64_to_str_length(value);

    // Place the null terminator at the end
    buffer[length - 1] = '\0';

    // Handle zero case explicitly
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    // Convert the number to a string
    for (UINTN i = length - 2; value > 0; i--) {
        buffer[i] = '0' + (value % 10);
        value /= 10;
    }
}

EFI_STATUS ExitBootServices(
	EFI_HANDLE* imageHandle,
	EFI_SYSTEM_TABLE* systemTable,
	EFI_MEMORY_DESCRIPTOR** memoryMap,
	UINTN* descriptorSize,
	UINTN* memoryMapSize)
{
	UINTN mapKey = 0;
	UINT32 descriptorVersion = 0;
	EFI_STATUS status = systemTable->BootServices->GetMemoryMap(
		memoryMapSize,
		*memoryMap,
		&mapKey,
		descriptorSize,
		&descriptorVersion);
	if(EFI_ERROR(status) && status != EFI_BUFFER_TOO_SMALL)
	{
		SN_LOG_ERROR(L"An unexpected error occured while trying to get the current memory map's size");
		return status;
	}

	// We need to allocate some more memory because the following allocation might change it
	*memoryMapSize += 2 * *descriptorSize;

	status = systemTable->BootServices->AllocatePool(
		EfiLoaderData,
		*memoryMapSize,
		(VOID**) memoryMap);
	if(EFI_ERROR(status))
	{
		SN_LOG_ERROR(L"An unexpected error occured while trying to allocate a buffer for the current memory map");
		return status;
	}

	status = systemTable->BootServices->GetMemoryMap(
		memoryMapSize,
		*memoryMap,
		&mapKey,
		descriptorSize,
		&descriptorVersion);
	if(EFI_ERROR(status))
	{
		SN_LOG_ERROR(L"An unexpecter error occured while trying to get the current memory map");
		return status;
	}

	status = systemTable->BootServices->ExitBootServices(
		imageHandle,
		mapKey);
	if(EFI_ERROR(status))
	{
		SN_LOG_ERROR(L"An unexpecter error occured while trying to exit boot services");
		return status;
	}

	return status;
}

