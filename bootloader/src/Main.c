#include "Kernel.h"
#include "Memory.h"
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

	UINT8* kernelFile = NULL;
	status = ReadFile(systemTable, rootVolume, L"Supernova\\Kernel.elf", (VOID**) &kernelFile);
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

	EFI_PHYSICAL_ADDRESS kernelP4Table;
	status = AllocateFrame(&frameAllocator, &kernelP4Table);
	if(EFI_ERROR(status))
	{
		goto halt;
	}

	status = InitEmptyPageTable(kernelP4Table);
	if(EFI_ERROR(status))
	{
		SN_LOG_ERROR(L"There was an error while trying to initialize an empty P4 Page Table");
		goto halt;
	}

	EFI_VIRTUAL_ADDRESS kernelEntryPoint;
	status = LoadKernel(kernelFile, &frameAllocator, kernelP4Table, &kernelEntryPoint);
	if(EFI_ERROR(status))
	{
		goto halt;
	}

	// TODO: So our problem now is that the kernel thinks its at physical address 100000,
	// where in reality its somewhere completely different.
	// I will have to change the allocator to start allocation at that address,
	// or recompile the kernel with a smaller physical address.
	//
	// TODO: Make another function to context switch, map it to the kernel's P4
	// so we don't page fault right out of the gate, load the P4 and jump to the entry address.
	//
	// This just might be all that is required for now, we'll see :D

halt:
	while(TRUE)
	{
		__asm__("cli; hlt");
	}

	// If we actually get here, smoething went catastrophically wrong 💀
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

