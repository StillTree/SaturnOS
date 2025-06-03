#include "ACPI.h"
#include "APIC.h"
#include "CPUInfo.h"
#include "Core.h"
#include "GDT.h"
#include "IDT.h"
#include "Logger.h"
#include "Memory/BitmapFrameAllocator.h"
#include "Memory/VirtualMemoryAllocator.h"
#include "PCI.h"
#include "Panic.h"
#include "Scheduler.h"
#include "Storage/Drivers/AHCI.h"
#include "Storage/Filesystems/Ext2.h"
#include "Storage/GPT.h"
#include "Syscalls.h"

#ifndef __x86_64__
#error SaturnKernel requires the x86 64-bit architecture to run properly!
#endif

/// Initially empty.
KernelBootInfo g_bootInfo = {};

void KernelMain(KernelBootInfo* bootInfo)
{
	// Copy the structure provided by the bootloader right at the beginning, so every part of the code can safely access it
	g_bootInfo = *bootInfo;

	// There is a guarantee that the bootloader will set up the framebuffer,
	// so this function doesn't throw but just warns when the serial output device is not available
	LoggerInit(&g_mainLogger, true, true, &g_bootInfo, 0x3f8);

	SK_LOG_INFO("Initializing the SaturnOS Kernel\n");

	SK_LOG_INFO("SaturnOS Copyright (C) 2024 StillTree (Alexander Debowski)");
	SK_LOG_INFO("This program comes with ABSOLUTELY NO WARRANTY; for details type ``.");
	SK_LOG_INFO("This is free software, and you are welcome to redistribute it");
	SK_LOG_INFO("under certain conditions; type `` for details.\n");

	SK_LOG_INFO("Saving the CPUID processor information");
	SK_PANIC_ON_ERROR(CPUIDSaveInfo(&g_cpuInformation), "Could not read the CPUID information");

	DisableInterrupts();

	// Doing that here, to not run into issues with unmasked interrupts or other bullshit later
	DisablePIC();

	SK_LOG_INFO("Initializing the GDT");
	InitGDT();
	SK_LOG_INFO("Initializing the IDT");
	InitIDT();

	EnableInterrupts();

	SK_LOG_INFO("Initializing the bitmap frame allocator");
	SK_PANIC_ON_ERROR(BitmapFrameAllocatorInit(&g_frameAllocator, (MemoryMapEntry*)g_bootInfo.MemoryMap, g_bootInfo.MemoryMapEntries),
		"Could not initialize the frame allocator");

	SK_LOG_DEBUG("Mapped Physical memory offset: 0x%x", g_bootInfo.PhysicalMemoryOffset);

	SK_LOG_INFO("Initializing the virtual memory allocator");
	SK_PANIC_ON_ERROR(InitKernelVirtualMemory(2, 0xffffff0000000000, 102400),
		"An unexpected error occured while trying to initialize the virtual memory allocator");

	SK_LOG_INFO("Parsing the ACPI structures");
	SK_PANIC_ON_ERROR(InitXSDT(), "An unexpected error occured while trying to parse ACPI structures");

	SK_LOG_INFO("Initializing the scheduler");
	InitSyscalls();
	InitScheduler();

	SK_LOG_INFO("Initializing the x2APIC");
	SK_PANIC_ON_ERROR(InitAPIC(), "An unexpected error occured while trying to initialize the APIC");

	SK_LOG_INFO("Scanning for available PCI devices");
	SK_PANIC_ON_ERROR(ScanPCIDevices(), "An unexpected error occured while trying to scan for available PCI devices");

	SK_PANIC_ON_ERROR(InitAHCI(), "An unexpected error occured while trying to initialize the AHCI driver");

	SK_PANIC_ON_ERROR(DetectGPTPartitions(), "An unexpected error occured while trying to detect GPT partitions");

	SK_PANIC_ON_ERROR(InitExt2(), "An unexpected error occured while trying to initialize the Ext2 driver");

	// TODO:
	// For the kernel reserve e.g. 2 PML4 entries and map them to some level 3 table, can be empty
	// Then this gives 1 TiB of usable virtual memory to do ASLR in and stuff
	// Adjust the memory manager to account for that
	//
	// For processes just copy these two PML4 entries into their PML4s
	// Remember to mark it as supervisor only and never change that
	// Never map anything for the kernel in the PML4s and onl use level 3 tables for mappings
	// Then on every new mapping I don't have to copy these changes to every single processes' PML4
	// since these 2 entries will stay unchanged, always pointing to the same level 3 tables and they will changes,
	// but since they are essentially "shared" I gain speed
	//
	// For userspace mapping in process PML4s it doesn't matter where I map, but definitely below the kernel
	// (in the lower canonical virtual memory region)

	while (true)
		__asm__ volatile("hlt");
}
