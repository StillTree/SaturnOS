#include "Uefi.h"
#include "Logger.h"
#include "FrameAllocator.h"
#include "EspFileSystem.h"

EFI_GUID gEfiGraphicsOutputProtocolGuid   = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
EFI_GUID gEfiSimpleTextOutProtocolGuid    = EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID;
EFI_GUID gEfiSimpleFileSystemProtocolGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
EFI_GUID gEfiLoadedImageProtocolGuid      = EFI_LOADED_IMAGE_PROTOCOL_GUID;

EFI_STATUS ExitBootServices(
	EFI_HANDLE* imageHandle,
	EFI_SYSTEM_TABLE* systemTable,
	EFI_MEMORY_DESCRIPTOR** memoryMap,
	UINTN* descriptorSize,
	UINTN* memoryMapSize);
EFI_STATUS OpenFileSystem(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable, EFI_FILE_PROTOCOL** rootVolume);

EFI_STATUS EFIAPI UefiMain(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable)
{
	(void) imageHandle;
	EFI_STATUS status = InitLogger(systemTable, &g_mainLogger, TRUE, TRUE);
	if(EFI_ERROR(status))
	{
		goto halt;
	}

	EFI_FILE_PROTOCOL* rootVolume = NULL;
	status = OpenFileSystem(imageHandle, systemTable, &rootVolume);
	if(EFI_ERROR(status))
	{
		goto halt;
	}

	UINT8* file = NULL;
	status = ReadFile(systemTable, rootVolume, L"Supernova\\zupa2.txt", (VOID**) &file);
	if(EFI_ERROR(status))
	{
		goto halt;
	}

	rootVolume->Close(rootVolume);

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

halt:
	while(TRUE)
	{
		__asm__("cli; hlt");
	}

	// We never ever want to get here after exiting boot services
	return EFI_SUCCESS;
}

EFI_STATUS OpenFileSystem(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable, EFI_FILE_PROTOCOL** rootVolume)
{
	EFI_LOADED_IMAGE_PROTOCOL* loadedImage = NULL;
	EFI_STATUS status = systemTable->BootServices->OpenProtocol(
		imageHandle,
		&gEfiLoadedImageProtocolGuid,
		(VOID**) &loadedImage,
		imageHandle,
		NULL,
		EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
	if(EFI_ERROR(status))
	{
		SN_LOG_ERROR(L"An unexpected error occured while trying to load the loaded image protocol");
		return status;
	}

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* simpleFileSystem = NULL;
	status = systemTable->BootServices->OpenProtocol(
		loadedImage->DeviceHandle,
		&gEfiSimpleFileSystemProtocolGuid,
		(VOID**) &simpleFileSystem,
		imageHandle,
		NULL,
		EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
	if(EFI_ERROR(status))
	{
		SN_LOG_ERROR(L"An unexpected error occured while trying to load the simple file system protocol");
		goto closeLoadedImage;
	}

	status = simpleFileSystem->OpenVolume(simpleFileSystem, rootVolume);
	if(EFI_ERROR(status))
	{
		SN_LOG_ERROR(L"An unexpected error occured while trynig to open the ESP handle");
		goto closeSimpleFileSystem;
	}

closeSimpleFileSystem:
	systemTable->BootServices->CloseProtocol(
		loadedImage->DeviceHandle,
		&gEfiSimpleFileSystemProtocolGuid,
		imageHandle,
		NULL);

closeLoadedImage:
	systemTable->BootServices->CloseProtocol(
		imageHandle,
		&gEfiLoadedImageProtocolGuid,
		imageHandle,
		NULL);

	return status;
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

