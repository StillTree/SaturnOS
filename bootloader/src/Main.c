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
VOID ContextSwitch(EFI_PHYSICAL_ADDRESS entryPoint, EFI_PHYSICAL_ADDRESS kernelP4Table, EFI_VIRTUAL_ADDRESS stackAddress);

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

	// TODO: Use some shit to determine the actual function size and if it needs 2 or even more pages to be mapped.
	EFI_PHYSICAL_ADDRESS contextSwitchFnAddress = (UINTN) ContextSwitch;
	status = MapMemoryPage(
		PhysFrameContainingAddress(contextSwitchFnAddress),
		PhysFrameContainingAddress(contextSwitchFnAddress),
		kernelP4Table,
		&frameAllocator,
		ENTRY_PRESENT | ENTRY_WRITEABLE);
	if(EFI_ERROR(status))
	{
		goto halt;
	}

	SN_LOG_INFO(L"Successfully identity-mapped the context switch function");

	EFI_VIRTUAL_ADDRESS virtualStackAddress = 0x8000000000;
	// Allocate 20 frames for the kernel's stack (80 KiB)
	for(UINTN i = 0; i < 21; i++)
	{
		EFI_PHYSICAL_ADDRESS frameAddress;
		status = AllocateFrame(&frameAllocator, &frameAddress);
		if(EFI_ERROR(status))
		{
			goto halt;
		}

		virtualStackAddress += 4096;
		status = MapMemoryPage(
			virtualStackAddress,
			frameAddress,
			kernelP4Table,
			&frameAllocator,
			ENTRY_PRESENT | ENTRY_WRITEABLE);
		if(EFI_ERROR(status))
		{
			goto halt;
		}
	}

	SN_LOG_INFO(L"Successfully allocated an 80 KiB kernel stack");
	SN_LOG_INFO(L"Performing context switch");

	ContextSwitch(kernelEntryPoint, kernelP4Table, virtualStackAddress - 4096);

halt:
	while(TRUE)
	{
		__asm__("cli; hlt");
	}

	// If we actually get here, smoething went catastrophically wrong 💀
	return EFI_SUCCESS;
}

VOID ContextSwitch(EFI_PHYSICAL_ADDRESS entryPoint, EFI_PHYSICAL_ADDRESS kernelP4Table, EFI_VIRTUAL_ADDRESS stackAddress)
{
	//__asm__ volatile("outb %b0, %w1" : : "a"('x'), "Nd"(0x3f8) : "memory");
	//__asm__ volatile("mov %0, %%cr3" : : "r"(kernelP4Table) : "memory");
	//__asm__ volatile("outb %b0, %w1" : : "a"('d'), "Nd"(0x3f8) : "memory");
	//__asm__ volatile("mov %0, %%rsp" : : "r"(stackAddress) : "memory");
	//__asm__ volatile("outb %b0, %w1" : : "a"('d'), "Nd"(0x3f8) : "memory");

	__asm__ volatile(
		"xor %%rbp, %%rbp\n\t"
		"mov %0, %%cr3\n\t"
		"mov %1, %%rsp\n\t"
		"push $0\n\t"
		"call *%2\n\t"
		"outb %b3, %w4\n\t"
		:
		: "r"(kernelP4Table),
		  "r"(stackAddress),
		  "r"(entryPoint),
		  "a"(':'), "Nd"(0x3f8)
		: "memory");

	//VOID (*KernelMain) (VOID) = (__attribute__((sysv_abi)) VOID(*) (VOID)) entryPoint;

	//__asm__ volatile("outb %b0, %w1" : : "a"('d'), "Nd"(0x3f8) : "memory");

	//KernelMain();
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

