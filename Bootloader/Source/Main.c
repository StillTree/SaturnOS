#include "ACPI.h"
#include "EspFileSystem.h"
#include "FrameAllocator.h"
#include "Kernel.h"
#include "Logger.h"
#include "Memory.h"
#include "MemoryMap.h"
#include "Ramdisk.h"
#include "Uefi.h"

EFI_GUID gEfiGraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
EFI_GUID gEfiSimpleTextOutProtocolGuid = EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID;
EFI_GUID gEfiSimpleFileSystemProtocolGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
EFI_GUID gEfiLoadedImageProtocolGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
EFI_GUID gEfiAcpi20TableGuid = EFI_ACPI_20_TABLE_GUID;

EFI_STATUS ExitBootServices(
	EFI_HANDLE* imageHandle, EFI_SYSTEM_TABLE* systemTable, EFI_MEMORY_DESCRIPTOR** memoryMap, UINTN* descriptorSize, UINTN* memoryMapSize);
EFI_STATUS OpenFileSystem(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable, EFI_FILE_PROTOCOL** rootVolume);
VOID ContextSwitch(EFI_PHYSICAL_ADDRESS entryPoint, EFI_PHYSICAL_ADDRESS kernelP4Table, EFI_VIRTUAL_ADDRESS stackAddress,
	EFI_VIRTUAL_ADDRESS bootInfoAddress);

EFI_STATUS EFIAPI UefiMain(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable)
{
	EFI_STATUS status = InitLogger(systemTable, &g_mainLogger, TRUE, TRUE);
	if (EFI_ERROR(status)) {
		goto halt;
	}

	EFI_FILE_PROTOCOL* rootVolume = NULL;
	status = OpenFileSystem(imageHandle, systemTable, &rootVolume);
	if (EFI_ERROR(status)) {
		goto halt;
	}

	UINT8* kernelFile = NULL;
	status = ReadFile(systemTable, rootVolume, L"Supernova\\Kernel.elf", (VOID**)&kernelFile, NULL);
	if (EFI_ERROR(status)) {
		goto halt;
	}

	UINT8* ramdiskFile = NULL;
	UINTN ramdiskFileSize = 0;
	status = ReadFile(systemTable, rootVolume, L"Supernova\\Ramdisk", (VOID**)&ramdiskFile, &ramdiskFileSize);
	if (EFI_ERROR(status)) {
		goto halt;
	}

	INT8* configFile = NULL;
	UINTN configFileSize = 0;
	status = ReadFile(systemTable, rootVolume, L"Supernova\\Config.conf", (VOID**)&configFile, &configFileSize);
	if (EFI_ERROR(status)) {
		goto halt;
	}

	rootVolume->Close(rootVolume);

	XSDP* xsdpPointer = NULL;
	status = FindXSDP(systemTable, (VOID**)&xsdpPointer);
	if (EFI_ERROR(status)) {
		goto halt;
	}

	UINTN descriptorSize = 0;
	UINTN memoryMapSize = 0;
	EFI_MEMORY_DESCRIPTOR* memoryMap = NULL;
	status = ExitBootServices(imageHandle, systemTable, &memoryMap, &descriptorSize, &memoryMapSize);
	if (EFI_ERROR(status)) {
		goto halt;
	}

	SN_LOG_INFO(L"Successfully exited boot services");

	FrameAllocatorData frameAllocator = { 0, 0, NULL, 0, 0 };
	status = InitFrameAllocator(&frameAllocator, memoryMap, memoryMapSize, descriptorSize);
	if (EFI_ERROR(status)) {
		goto halt;
	}

	EFI_PHYSICAL_ADDRESS kernelP4Table;
	status = AllocateFrame(&frameAllocator, &kernelP4Table);
	if (EFI_ERROR(status)) {
		goto halt;
	}

	status = InitEmptyPageTable(kernelP4Table);
	if (EFI_ERROR(status)) {
		SN_LOG_ERROR(L"There was an error while trying to initialize an empty P4 Page Table");
		goto halt;
	}

	// Every kernel "thing" will be allocated one after another right after the kernel itself
	EFI_VIRTUAL_ADDRESS nextUsableVirtualPage;
	EFI_VIRTUAL_ADDRESS kernelEntryPoint;
	status = LoadKernel(kernelFile, &frameAllocator, kernelP4Table, &kernelEntryPoint, &nextUsableVirtualPage);
	if (EFI_ERROR(status)) {
		goto halt;
	}

	SN_LOG_INFO(L"Successfully loaded the kernel executable into memory");

	EFI_VIRTUAL_ADDRESS kernelArgsVirtualAddress = nextUsableVirtualPage;
	status = LoadKernelArgs(configFile, configFileSize, &frameAllocator, kernelP4Table, &nextUsableVirtualPage);
	if (EFI_ERROR(status)) {
		goto halt;
	}

	EFI_VIRTUAL_ADDRESS ramdiskVirtualAddress = nextUsableVirtualPage;
	status = LoadRamdisk(ramdiskFile, ramdiskFileSize, &frameAllocator, kernelP4Table, &nextUsableVirtualPage);
	if (EFI_ERROR(status)) {
		goto halt;
	}

	// TODO: Use some shit to determine the actual function size and if it needs 2 or even more pages to be mapped.
	EFI_PHYSICAL_ADDRESS contextSwitchFnAddress = (UINTN)ContextSwitch;

	status = MapMemoryPage4KiB(PhysFrameContainingAddress(contextSwitchFnAddress), PhysFrameContainingAddress(contextSwitchFnAddress),
		kernelP4Table, &frameAllocator, ENTRY_PRESENT | ENTRY_WRITEABLE);
	if (EFI_ERROR(status)) {
		goto halt;
	}

	SN_LOG_INFO(L"Successfully identity-mapped the context switch function");

	// We need a quard page right after the stack to fault when overflowing
	nextUsableVirtualPage += 4096;
	// Allocate 25 frames for the kernel's stack (100 KiB)
	for (UINTN i = 0; i < 25; i++) {
		EFI_PHYSICAL_ADDRESS frameAddress;
		status = AllocateFrame(&frameAllocator, &frameAddress);
		if (EFI_ERROR(status)) {
			goto halt;
		}

		status = MapMemoryPage4KiB(
			nextUsableVirtualPage, frameAddress, kernelP4Table, &frameAllocator, ENTRY_PRESENT | ENTRY_WRITEABLE | ENTRY_NO_EXECUTE);
		if (EFI_ERROR(status)) {
			goto halt;
		}

		nextUsableVirtualPage += 4096;
	}

	EFI_VIRTUAL_ADDRESS stackVirtualAddress = nextUsableVirtualPage;

	SN_LOG_INFO(L"Successfully allocated a 100 KiB kernel stack");

	// Allocate the boot info right after the stack
	EFI_VIRTUAL_ADDRESS bootInfoVirtualAddress = nextUsableVirtualPage;
	EFI_PHYSICAL_ADDRESS bootInfoPhysicalAddress;
	status = AllocateFrame(&frameAllocator, &bootInfoPhysicalAddress);
	if (EFI_ERROR(status)) {
		goto halt;
	}

	status = MapMemoryPage4KiB(bootInfoVirtualAddress, bootInfoPhysicalAddress, kernelP4Table, &frameAllocator,
		ENTRY_PRESENT | ENTRY_WRITEABLE | ENTRY_NO_EXECUTE);
	if (EFI_ERROR(status)) {
		goto halt;
	}

	KernelBootInfo* bootInfo = (KernelBootInfo*)bootInfoPhysicalAddress;
	bootInfo->kernelArgsAddress = kernelArgsVirtualAddress;
	bootInfo->ramdiskAddress = ramdiskVirtualAddress;
	bootInfo->ramdiskSizeBytes = ramdiskFileSize;
	bootInfo->contextSwitchFunctionPage = PhysFrameContainingAddress(contextSwitchFnAddress);
	bootInfo->kernelAddress = 0xffffffff80010000;
	bootInfo->kernelSize = nextUsableVirtualPage + 4096 - bootInfo->kernelAddress;
	bootInfo->kernelStackTop = stackVirtualAddress;
	bootInfo->xsdtAddress = (EFI_PHYSICAL_ADDRESS)xsdpPointer->XsdtAddress;
	bootInfo->framebufferSize = g_mainLogger.framebuffer.framebufferSize;
	bootInfo->framebufferWidth = g_mainLogger.framebuffer.width;
	bootInfo->framebufferHeight = g_mainLogger.framebuffer.height;
	bootInfo->kernelP4TableAddress = kernelP4Table;

	// I don't know if the framebuffer is guaranteed to be page aligned (4096 bytes), so after aligning its physical beginning to the
	// containing frame I get the value contained in the least significant 12 bits of the address (the page offset) and add it back to the
	// virtual address if the framebuffer happens to not be exactly 4096 bytes aligned
	UINTN framebufferPages = (g_mainLogger.framebuffer.framebufferSize + 4095) / 4096;
	UINTN framebufferPhysicalAddress = PhysFrameContainingAddress((UINTN)g_mainLogger.framebuffer.framebuffer);
	// We want the framebuffer's virtual address to be right after boot info's virtual address
	UINTN framebufferVirtualAddress = bootInfoVirtualAddress + 4096;
	UINTN framebufferFrameOffset = (UINTN)g_mainLogger.framebuffer.framebuffer & 0xfff;
	bootInfo->framebufferAddress = framebufferVirtualAddress + framebufferFrameOffset;
	for (UINTN i = 0; i < framebufferPages; i++) {
		status = MapMemoryPage4KiB(framebufferVirtualAddress, framebufferPhysicalAddress, kernelP4Table, &frameAllocator,
			ENTRY_PRESENT | ENTRY_WRITEABLE | ENTRY_NO_EXECUTE);
		if (EFI_ERROR(status)) {
			goto halt;
		}

		framebufferPhysicalAddress += 4096;
		framebufferVirtualAddress += 4096;
	}

	bootInfo->physicalMemoryMappingSize = 1073741824; // 1 GiB
	// Round the next available virtual address to the nearest 2 MiB
	bootInfo->physicalMemoryOffset = (framebufferVirtualAddress + 0x1FFFFF) & ~0x1FFFFF;
	// Mapping the whole (512 huge pages for now) physical memory at an offset using 2 MiB huge pages
	EFI_VIRTUAL_ADDRESS mappingOffset = bootInfo->physicalMemoryOffset;
	for (UINT64 i = 0; i < 512; i++) {
		status = MapMemoryPage2MiB(
			mappingOffset, 0x200000 * i, kernelP4Table, &frameAllocator, ENTRY_PRESENT | ENTRY_WRITEABLE | ENTRY_HUGE_PAGE);
		if (EFI_ERROR(status)) {
			goto halt;
		}

		mappingOffset += 0x200000;
	}

	// After this function call, no other frame allocations should be performed
	UINTN kernelMemoryMapEntries = 0;
	status = CreateMemoryMap(&frameAllocator, kernelP4Table, &kernelMemoryMapEntries, memoryMap, memoryMapSize, descriptorSize,
		mappingOffset); // The memory map should be allocated right after the physical memory mapping
	if (EFI_ERROR(status)) {
		goto halt;
	}
	bootInfo->memoryMapAddress = mappingOffset;
	bootInfo->memoryMapEntries = kernelMemoryMapEntries;

	SN_LOG_INFO(L"Performing context switch");

	ContextSwitch(kernelEntryPoint, kernelP4Table,
		stackVirtualAddress, // x86_64 System V ABI requires the stack to be 16 bit aligned so I hope that it is :D
		bootInfoVirtualAddress);

halt:
	while (TRUE) {
		__asm__("cli; hlt");
	}

	// If we ever actually get here, smoething went catastrophically wrong 💀
	return EFI_SUCCESS;
}

__attribute__((noreturn)) VOID ContextSwitch(EFI_PHYSICAL_ADDRESS entryPoint, EFI_PHYSICAL_ADDRESS kernelP4Table,
	EFI_VIRTUAL_ADDRESS stackAddress, EFI_VIRTUAL_ADDRESS bootInfoAddress)
{
	__asm__ volatile("xor %%rbp, %%rbp\n\t" // Zero out the base pointer (a qequirement when loading ELF64 files)
					 "mov %0, %%cr3\n\t" // Load the P4's address - address space switch
					 "mov %1, %%rsp\n\t" // We want a new stack so we load it into the stack pointer
					 "mov %2, %%rdi\n\t" // The first function argument is a pointer to the boot info
					 "push $0\n\t"
					 "call *%3\n\t" // Call the kernel function
					 "outb %b4, %w5\n\t" // If we exit the kernel's function which shouldn't happen, we print "^" to the COM1 serial output
		:
		: "r"(kernelP4Table), "r"(stackAddress), "r"(bootInfoAddress), "r"(entryPoint), "a"('^'), "Nd"(0x3f8)
		: "memory");

	// We shouldn't ever exit this function...
	while (TRUE)
		;
}

EFI_STATUS OpenFileSystem(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable, EFI_FILE_PROTOCOL** rootVolume)
{
	EFI_LOADED_IMAGE_PROTOCOL* loadedImage = NULL;
	EFI_STATUS status = systemTable->BootServices->OpenProtocol(
		imageHandle, &gEfiLoadedImageProtocolGuid, (VOID**)&loadedImage, imageHandle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
	if (EFI_ERROR(status)) {
		SN_LOG_ERROR(L"An unexpected error occured while trying to load the loaded image protocol");
		return status;
	}

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* simpleFileSystem = NULL;
	status = systemTable->BootServices->OpenProtocol(loadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid,
		(VOID**)&simpleFileSystem, imageHandle, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
	if (EFI_ERROR(status)) {
		SN_LOG_ERROR(L"An unexpected error occured while trying to load the simple file system protocol");
		goto closeLoadedImage;
	}

	status = simpleFileSystem->OpenVolume(simpleFileSystem, rootVolume);
	if (EFI_ERROR(status)) {
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
	EFI_HANDLE* imageHandle, EFI_SYSTEM_TABLE* systemTable, EFI_MEMORY_DESCRIPTOR** memoryMap, UINTN* descriptorSize, UINTN* memoryMapSize)
{
	UINTN mapKey = 0;
	UINT32 descriptorVersion = 0;
	EFI_STATUS status = systemTable->BootServices->GetMemoryMap(memoryMapSize, *memoryMap, &mapKey, descriptorSize, &descriptorVersion);
	if (EFI_ERROR(status) && status != EFI_BUFFER_TOO_SMALL) {
		SN_LOG_ERROR(L"An unexpected error occured while trying to get the current memory map's size");
		return status;
	}

	// We need to allocate some more memory because the following allocation might
	// change it
	*memoryMapSize += 2 * *descriptorSize;

	status = systemTable->BootServices->AllocatePool(EfiLoaderData, *memoryMapSize, (VOID**)memoryMap);
	if (EFI_ERROR(status)) {
		SN_LOG_ERROR(L"An unexpected error occured while trying to allocate a buffer for the current memory map");
		return status;
	}

	status = systemTable->BootServices->GetMemoryMap(memoryMapSize, *memoryMap, &mapKey, descriptorSize, &descriptorVersion);
	if (EFI_ERROR(status)) {
		SN_LOG_ERROR(L"An unexpecter error occured while trying to get the current memory map");
		return status;
	}

	status = systemTable->BootServices->ExitBootServices(imageHandle, mapKey);
	if (EFI_ERROR(status)) {
		SN_LOG_ERROR(L"An unexpecter error occured while trying to exit boot services");
		return status;
	}

	return status;
}
