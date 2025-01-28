#include "ACPI.h"
#include "APIC.h"
#include "CPUID.h"
#include "Core.h"
#include "GDT.h"
#include "IDT.h"
#include "Logger.h"
#include "MSR.h"
#include "Memory/BitmapFrameAllocator.h"
#include "Memory/HeapMemoryAllocator.h"
#include "PCI.h"
#include "Result.h"
#include "Storage/Drivers/NVMe.h"

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
		SK_LOG_ERROR("Could not read the CPUID information");
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
		SK_LOG_ERROR("Could not initialize the frame allocator");
	}

	SK_LOG_DEBUG("Mapped Physical memory offset: %x", g_bootInfo.PhysicalMemoryOffset);

	SK_LOG_INFO("Initializing the kernel's memory heap");
	result = HeapInit(&g_heapMemoryAllocator, 102400, 0x6969'6969'0000);
	if (result) {
		SK_LOG_ERROR("Could not initialize the kernel's heap memory pool");
	}

	SK_LOG_INFO("Parsing the ACPI structures");
	result = InitXSDT();
	if (result) {
		SK_LOG_ERROR("An unexpected error occured while trying to parse ACPI structures");
	}

	SK_LOG_INFO("Initializing the x2APIC");
	result = InitAPIC();
	if (result) {
		SK_LOG_ERROR("An unexpected error occured while trying to initialize the x2APIC");
	}

	SK_LOG_INFO("Scanning for available PCI devices");
	result = ScanPCIDevices();
	if (result) {
		SK_LOG_ERROR("An unexpected error occured while trying to scan for available PCI devices");
	}

	SK_LOG_INFO("Initializing the NVMe storage driver");
	result = NVMeInit(&g_nvmeDriver);
	if (result) {
		SK_LOG_ERROR("An unexpected error occured while initializing the NVMe storage driver");
	}

	PhysicalAddress identifyBuffer;
	AllocateFrame(&g_frameAllocator, &identifyBuffer);
	NVMeSubmissionEntry e = {};
	e.CDW0 = 0x6;
	e.PRP1 = identifyBuffer;
	e.CDW10 = 1;
	e.NSID = 0;

	NVMeSendAdminCommand(&g_nvmeDriver, &e);

	NVMeCompletionEntry c = {};
	result = NVMePollNextAdminCompletion(&g_nvmeDriver, &c);
	if (!result) {
		SK_LOG_INFO("!!! NICE !!!");

		u16* a = (u16*)(identifyBuffer + g_bootInfo.PhysicalMemoryOffset);
		SK_LOG_INFO("%x", *a);
	}

	// __asm__ volatile("int3");

	while (true)
		__asm__("hlt");
}
