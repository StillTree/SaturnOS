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
#include "Storage/Filesystems/STFS.h"
#include "Storage/GPT.h"
#include "Storage/VirtualFileSystem.h"
#include "Syscalls.h"

#ifndef __x86_64__
#error SaturnKernel only supports the x86 64-bit architecture!
#endif

/// Initially empty.
KernelBootInfo g_bootInfo = {};

void KernelMain(KernelBootInfo* bootInfo)
{
	// Copy the structure provided by the bootloader right at the beginning, so every part of the code can safely access it
	g_bootInfo = *bootInfo;

	// There is a guarantee that the bootloader will set up the framebuffer,
	// so this function doesn't throw but just warns when the serial output device is not available
	LoggerInit(true, true, &g_bootInfo, 0x3f8);

	Log(SK_LOG_INFO "Initializing the SaturnOS Kernel\n");

	Log(SK_LOG_INFO "SaturnOS Copyright (C) 2025 StillTree (Alexander Debowski)");
	Log(SK_LOG_INFO "This program comes with ABSOLUTELY NO WARRANTY; for details type ``.");
	Log(SK_LOG_INFO "This is free software, and you are welcome to redistribute it");
	Log(SK_LOG_INFO "under certain conditions; type `` for details.\n");

	Log(SK_LOG_INFO "Saving the CPUID processor information");
	SK_PANIC_ON_ERROR(CPUIDSaveInfo(&g_cpuInformation), "Could not read the CPUID information");

	DisableInterrupts();

	// Doing that here, to not run into issues with unmasked interrupts or other bullshit later
	DisablePIC();

	Log(SK_LOG_INFO "Initializing the GDT");
	InitGDT();
	Log(SK_LOG_INFO "Initializing the IDT");
	InitIDT();

	EnableInterrupts();

	Log(SK_LOG_INFO "Initializing the bitmap frame allocator");
	SK_PANIC_ON_ERROR(BitmapFrameAllocatorInit(&g_frameAllocator, (MemoryMapEntry*)g_bootInfo.MemoryMap, g_bootInfo.MemoryMapEntries),
		"Could not initialize the frame allocator");

	Log(SK_LOG_DEBUG "Mapped Physical memory offset: 0x%x", g_bootInfo.PhysicalMemoryOffset);

	Log(SK_LOG_INFO "Initializing the virtual memory allocator");
	SK_PANIC_ON_ERROR(InitKernelVirtualMemory(2, 0xffffff0000000000, 102400),
		"An unexpected error occured while trying to initialize the virtual memory allocator");

	Log(SK_LOG_INFO "Parsing the ACPI structures");
	SK_PANIC_ON_ERROR(InitXSDT(), "An unexpected error occured while trying to parse ACPI structures");

	Log(SK_LOG_INFO "Initializing the scheduler");
	InitSyscalls();
	InitScheduler(&g_scheduler);

	Log(SK_LOG_INFO "Initializing the x2APIC");
	SK_PANIC_ON_ERROR(InitAPIC(), "An unexpected error occured while trying to initialize the APIC");

	Log(SK_LOG_INFO "Initializing the virtual file system layer");
	SK_PANIC_ON_ERROR(InitVirtualFileSystem(&g_virtualFileSystem),
		"An unexpected error occured while trying to initialize the virtual file system layer");

	Log(SK_LOG_INFO "Initializing the STFS ramdisk builtin driver");
	SK_PANIC_ON_ERROR(InitSTFS(), "An unexpected error occured while trying to initialize the STFS ramdisk builtin driver");

	Log(SK_LOG_INFO "Scanning for available PCI devices");
	SK_PANIC_ON_ERROR(ScanPCIDevices(), "An unexpected error occured while trying to scan for available PCI devices");

	SK_PANIC_ON_ERROR(InitAHCI(), "An unexpected error occured while trying to initialize the AHCI driver");

	SK_PANIC_ON_ERROR(DetectGPTPartitions(), "An unexpected error occured while trying to detect GPT partitions");

	SK_PANIC_ON_ERROR(InitExt2(), "An unexpected error occured while trying to initialize the Ext2 driver");

	while (true)
		__asm__ volatile("hlt");
}
