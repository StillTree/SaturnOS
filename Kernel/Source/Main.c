#include "ACPI.h"
#include "APIC.h"
#include "CPUInfo.h"
#include "Core.h"
#include "GDT.h"
#include "IDT.h"
#include "Logger.h"
#include "MSR.h"
#include "Memory/BitmapFrameAllocator.h"
#include "Memory/HeapMemoryAllocator.h"
#include "PCI.h"
#include "Result.h"
#include "Storage/Drivers/AHCI.h"
#include "Storage/GPT.h"
#include "Storage/Filesystems/Ext2.h"
#include "Scheduler.h"

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
	Result result = CPUIDSaveInfo(&g_cpuInformation);
	if (result) {
		SK_LOG_ERROR("Could not read the CPUID information: %r", result);
		goto halt;
	}

	DisableInterrupts();

	// Doing that here, to not run into issues with unmasked interrupts or other bullshit later
	DisablePIC();

	SK_LOG_INFO("Initializing the GDT");
	InitGDT();
	SK_LOG_INFO("Initializing the IDT");
	InitIDT();

	EnableInterrupts();

	SK_LOG_INFO("Initializing the bitmap frame allocator");
	result = BitmapFrameAllocatorInit(&g_frameAllocator, (MemoryMapEntry*)g_bootInfo.MemoryMap, g_bootInfo.MemoryMapEntries);
	if (result) {
		SK_LOG_ERROR("Could not initialize the frame allocator: %r", result);
		goto halt;
	}

	SK_LOG_DEBUG("Mapped Physical memory offset: %x", g_bootInfo.PhysicalMemoryOffset);

	SK_LOG_INFO("Initializing the kernel's memory heap");
	result = HeapInit(&g_heapMemoryAllocator, 102400, 0x6969'6969'0000);
	if (result) {
		SK_LOG_ERROR("Could not initialize the kernel's heap memory pool: %r", result);
		goto halt;
	}

	SK_LOG_INFO("Parsing the ACPI structures");
	result = InitXSDT();
	if (result) {
		SK_LOG_ERROR("An unexpected error occured while trying to parse ACPI structures: %r", result);
		goto halt;
	}

	SK_LOG_INFO("Initializing the scheduler");
	result = InitScheduler();
	if (result) {
		SK_LOG_ERROR("An unexpected error occured while trying to initialize the scheduler: %r", result);
		goto halt;
	}

	SK_LOG_INFO("Initializing the x2APIC");
	result = InitAPIC();
	if (result) {
		SK_LOG_ERROR("An unexpected error occured while trying to initialize the APIC: %r", result);
		goto halt;
	}

	SK_LOG_INFO("Scanning for available PCI devices");
	result = ScanPCIDevices();
	if (result) {
		SK_LOG_ERROR("An unexpected error occured while trying to scan for available PCI devices: %r", result);
		goto halt;
	}

	result = InitAHCI();
	if (result) {
		SK_LOG_ERROR("An unexpected error occured while trying to initialize the AHCI driver: %r", result);
		goto halt;
	}

	result = DetectGPTPartitions();
	if (result) {
		SK_LOG_ERROR("An unexpected error occured while trying to detect GPT partitions: %r", result);
		goto halt;
	}

	result = InitExt2();
	if (result) {
		SK_LOG_ERROR("An unexpected error occured while trying to initialize the Ext2 driver: %r", result);
		goto halt;
	}

halt:
	while (true)
		__asm__("hlt");
}
