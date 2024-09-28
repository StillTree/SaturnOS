#include "EspFileSystem.h"
#include "FrameAllocator.h"
#include "Kernel.h"
#include "Logger.h"
#include "Memory.h"
#include "MemoryMap.h"
#include "Uefi.h"

EFI_GUID gEfiGraphicsOutputProtocolGuid	  = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
EFI_GUID gEfiSimpleTextOutProtocolGuid	  = EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID;
EFI_GUID gEfiSimpleFileSystemProtocolGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
EFI_GUID gEfiLoadedImageProtocolGuid	  = EFI_LOADED_IMAGE_PROTOCOL_GUID;

EFI_STATUS ExitBootServices(
	EFI_HANDLE* imageHandle,
	EFI_SYSTEM_TABLE* systemTable,
	EFI_MEMORY_DESCRIPTOR** memoryMap,
	UINTN* descriptorSize,
	UINTN* memoryMapSize);
EFI_STATUS OpenFileSystem(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable, EFI_FILE_PROTOCOL** rootVolume);
VOID ContextSwitch(
	EFI_PHYSICAL_ADDRESS entryPoint,
	EFI_PHYSICAL_ADDRESS kernelP4Table,
	EFI_VIRTUAL_ADDRESS stackAddress,
	EFI_VIRTUAL_ADDRESS bootInfoAddress);

EFI_STATUS EFIAPI UefiMain(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable)
{
	EFI_STATUS status = InitLogger(systemTable, &g_mainLogger, TRUE, TRUE);
	if(EFI_ERROR(status))
	{
		goto halt;
	}

	EFI_FILE_PROTOCOL* rootVolume = NULL;
	status						  = OpenFileSystem(imageHandle, systemTable, &rootVolume);
	if(EFI_ERROR(status))
	{
		goto halt;
	}

	UINT8* kernelFile = NULL;
	status			  = ReadFile(systemTable, rootVolume, L"Supernova\\Kernel.elf", (VOID**) &kernelFile);
	if(EFI_ERROR(status))
	{
		goto halt;
	}

	rootVolume->Close(rootVolume);

	UINTN descriptorSize			 = 0;
	UINTN memoryMapSize				 = 0;
	EFI_MEMORY_DESCRIPTOR* memoryMap = NULL;
	status							 = ExitBootServices(imageHandle, systemTable, &memoryMap, &descriptorSize, &memoryMapSize);
	if(EFI_ERROR(status))
	{
		goto halt;
	}

	SN_LOG_INFO(L"Successfully exited boot services");

	FrameAllocatorData frameAllocator = { 0, 0, NULL, 0, 0 };
	status							  = InitFrameAllocator(&frameAllocator, memoryMap, memoryMapSize, descriptorSize);
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
		SN_LOG_ERROR(L"There was an error while trying to initialize an empty P4 "
					 L"Page Table");
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

	status = MapMemoryPage4KiB(
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

	// We need a guard page at the end of the stack to page fault when overflowing
	EFI_VIRTUAL_ADDRESS virtualStackAddress = 0x8000001000;
	// Allocate 20 frames for the kernel's stack (80 KiB)
	for(UINTN i = 0; i < 20; i++)
	{
		EFI_PHYSICAL_ADDRESS frameAddress;
		status = AllocateFrame(&frameAllocator, &frameAddress);
		if(EFI_ERROR(status))
		{
			goto halt;
		}

		status = MapMemoryPage4KiB(
			virtualStackAddress,
			frameAddress,
			kernelP4Table,
			&frameAllocator,
			ENTRY_PRESENT | ENTRY_WRITEABLE | ENTRY_NO_EXECUTE);
		if(EFI_ERROR(status))
		{
			goto halt;
		}

		virtualStackAddress += 4096;
	}

	SN_LOG_INFO(L"Successfully allocated an 80 KiB kernel stack");

	// Allocate the boot info right after the stack
	EFI_VIRTUAL_ADDRESS bootInfoVirtualAddress = virtualStackAddress;
	EFI_PHYSICAL_ADDRESS bootInfoPhysicalAddress;
	status = AllocateFrame(&frameAllocator, &bootInfoPhysicalAddress);
	if(EFI_ERROR(status))
	{
		goto halt;
	}

	status = MapMemoryPage4KiB(
		bootInfoVirtualAddress,
		bootInfoPhysicalAddress,
		kernelP4Table,
		&frameAllocator,
		ENTRY_PRESENT | ENTRY_WRITEABLE | ENTRY_NO_EXECUTE);
	if(EFI_ERROR(status))
	{
		goto halt;
	}

	KernelBootInfo* bootInfo	 = (KernelBootInfo*) bootInfoPhysicalAddress;
	bootInfo->framebufferSize	 = g_mainLogger.framebuffer.framebufferSize;
	bootInfo->framebufferAddress = (UINTN) g_mainLogger.framebuffer.framebuffer;
	bootInfo->framebufferWidth	 = g_mainLogger.framebuffer.width;
	bootInfo->framebufferHeight	 = g_mainLogger.framebuffer.height;

	UINTN framebufferPages			 = (bootInfo->framebufferSize + 4095) / 4096;
	UINTN framebufferPhysicalAddress = bootInfo->framebufferAddress;
	for(UINTN i = 0; i < framebufferPages; i++)
	{
		status = MapMemoryPage4KiB(
			framebufferPhysicalAddress,
			framebufferPhysicalAddress,
			kernelP4Table,
			&frameAllocator,
			ENTRY_PRESENT | ENTRY_WRITEABLE | ENTRY_NO_EXECUTE);
		if(EFI_ERROR(status))
		{
			goto halt;
		}

		framebufferPhysicalAddress += 4096;
	}

	// Mapping the whole (512 pages for now = 1 GiB) physical memory at an offset
	// using 2MiB huge pages
	const EFI_VIRTUAL_ADDRESS mappingOffset = 0xA0000000000; // 10 TiB in hex (I think...)
	for(UINT64 i = 0; i < 512; i++)
	{
		status = MapMemoryPage2MiB(
			mappingOffset + 0x200000 * i,
			0x200000 * i,
			kernelP4Table,
			&frameAllocator,
			ENTRY_PRESENT | ENTRY_WRITEABLE | ENTRY_HUGE_PAGE);
		if(EFI_ERROR(status))
		{
			goto halt;
		}
	}

	// After this function call no other frame allocations should be made
	MemoryMapEntry* kernelMemoryMap = NULL;
	UINTN kernelMemoryMapEntries	= 0;
	status							= CreateMemoryMap(
		 &frameAllocator,
		 kernelP4Table,
		 &kernelMemoryMap,
		 &kernelMemoryMapEntries,
		 memoryMap,
		 memoryMapSize,
		 descriptorSize,
		 bootInfoVirtualAddress + 4096);
	if(EFI_ERROR(status))
	{
		goto halt;
	}
	bootInfo->memoryMapAddress = bootInfoVirtualAddress + 4096;
	bootInfo->memoryMapEntries = kernelMemoryMapEntries;

	SN_LOG_INFO(L"Performing context switch");

	ContextSwitch(
		kernelEntryPoint,
		kernelP4Table,
		virtualStackAddress, // x86_64 System V ABI requires the stack to be 16
							 // bit aligned so it hopefully is :D
		bootInfoVirtualAddress);

halt:
	while(TRUE)
	{
		__asm__("cli; hlt");
	}

	// If we actually get here, smoething went catastrophically wrong 💀
	return EFI_SUCCESS;
}

VOID ContextSwitch(
	EFI_PHYSICAL_ADDRESS entryPoint,
	EFI_PHYSICAL_ADDRESS kernelP4Table,
	EFI_VIRTUAL_ADDRESS stackAddress,
	EFI_VIRTUAL_ADDRESS bootInfoAddress)
{
	__asm__ volatile("xor %%rbp, %%rbp\n\t" // Zero out the base pointer, the kernel will take
											// care of it from here
					 "mov %0, %%cr3\n\t"	// Load the P4's address - address space switch
					 "mov %1, %%rsp\n\t"	// We want a new stack so we load it into the stack
											// pointer
					 "mov %2, %%rdi\n\t"	// The first function argument is a pointer to the
											// boot info
					 "push $0\n\t"			// To be honest, I don't know if that even does something
					 "call *%3\n\t"			// Call the kernel function
					 "outb %b4, %w5\n\t"	// If we exit the kernel's function which shouldn't
											// happen, we print a colon to the COM1 serial output
					 :
					 : "r"(kernelP4Table), "r"(stackAddress), "r"(bootInfoAddress), "r"(entryPoint), "a"(':'), "Nd"(0x3f8)
					 : "memory");

	// We should not ever exit this function...
}

EFI_STATUS OpenFileSystem(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable, EFI_FILE_PROTOCOL** rootVolume)
{
	EFI_LOADED_IMAGE_PROTOCOL* loadedImage = NULL;
	EFI_STATUS status					   = systemTable->BootServices->OpenProtocol(
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
	status											  = systemTable->BootServices->OpenProtocol(
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
	systemTable->BootServices->CloseProtocol(loadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, imageHandle, NULL);

closeLoadedImage:
	systemTable->BootServices->CloseProtocol(imageHandle, &gEfiLoadedImageProtocolGuid, imageHandle, NULL);

	return status;
}

EFI_STATUS ExitBootServices(
	EFI_HANDLE* imageHandle,
	EFI_SYSTEM_TABLE* systemTable,
	EFI_MEMORY_DESCRIPTOR** memoryMap,
	UINTN* descriptorSize,
	UINTN* memoryMapSize)
{
	UINTN mapKey			 = 0;
	UINT32 descriptorVersion = 0;
	EFI_STATUS status = systemTable->BootServices->GetMemoryMap(memoryMapSize, *memoryMap, &mapKey, descriptorSize, &descriptorVersion);
	if(EFI_ERROR(status) && status != EFI_BUFFER_TOO_SMALL)
	{
		SN_LOG_ERROR(L"An unexpected error occured while trying to get the current memory map's size");
		return status;
	}

	// We need to allocate some more memory because the following allocation might
	// change it
	*memoryMapSize += 2 * *descriptorSize;

	status = systemTable->BootServices->AllocatePool(EfiLoaderData, *memoryMapSize, (VOID**) memoryMap);
	if(EFI_ERROR(status))
	{
		SN_LOG_ERROR(L"An unexpected error occured while trying to allocate a buffer for the current memory map");
		return status;
	}

	status = systemTable->BootServices->GetMemoryMap(memoryMapSize, *memoryMap, &mapKey, descriptorSize, &descriptorVersion);
	if(EFI_ERROR(status))
	{
		SN_LOG_ERROR(L"An unexpecter error occured while trying to get the current memory map");
		return status;
	}

	status = systemTable->BootServices->ExitBootServices(imageHandle, mapKey);
	if(EFI_ERROR(status))
	{
		SN_LOG_ERROR(L"An unexpecter error occured while trying to exit boot services");
		return status;
	}

	return status;
}
