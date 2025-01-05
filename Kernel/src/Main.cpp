#include "Core.hpp"

#include "ACPI.hpp"
#include "CPUID.hpp"
#include "GDT.hpp"
#include "IDT.hpp"
#include "Logger.hpp"
#include "Memory/BitmapFrameAllocator.hpp"
#include "Memory/HeapMemoryAllocator.hpp"
#include "PIC.hpp"
#include "Result.hpp"

#ifndef __x86_64__
#error SaturnKernel requires the x86 64-bit architecture to run properly!
#endif

// Initially empty.
SaturnKernel::KernelBootInfo SaturnKernel::g_bootInfo = {};

/// C linking so the linker and the bootloader don't absolutely shit themselves.
extern "C" auto KernelMain(SaturnKernel::KernelBootInfo* bootInfo) -> void
{
	using namespace SaturnKernel;

	// Copy the structure provided by the bootloader right at the beginning, so every part of the code can safely access it.
	g_bootInfo = *bootInfo;

	// There is a guarantee that the bootloader will set up the framebuffer,
	// so this function doesn't throw but just warns when the serial output device is not available.
	g_mainLogger.Init(true, true, g_bootInfo, 0x3f8);

	SK_LOG_INFO("Initializing the SaturnOS Kernel\n");

	SK_LOG_INFO("SaturnOS Copyright (C) 2024 StillTree (Alexander Debowski)");
	SK_LOG_INFO("This program comes with ABSOLUTELY NO WARRANTY; for details type ``.");
	SK_LOG_INFO("This is free software, and you are welcome to redistribute it");
	SK_LOG_INFO("under certain conditions; type `` for details.\n");

	DisableInterrupts();

	SK_LOG_INFO("Initializing the GDT");
	InitGDT();
	SK_LOG_INFO("Initializing the IDT");
	InitIDT();

	SK_LOG_INFO("Initializing the Intel PIC 8259");
	ReinitializePIC();

	EnableInterrupts();

	SK_LOG_INFO("Saving the CPUID processor information");
	auto result = g_cpuInformation.SaveInfo();
	if (result.IsError()) {
		SK_LOG_ERROR("Could not read the CPUID information");
	}

	SK_LOG_INFO("Initializing the sequential frame allocator");
	result = g_frameAllocator.Init(static_cast<MemoryMapEntry*>(g_bootInfo.MemoryMap), g_bootInfo.MemoryMapEntries);
	if (result.IsError()) {
		SK_LOG_ERROR("Could not initialize the frame allocator");
	}

	SK_LOG_DEBUG("Mapped Physical memory offset: {}", g_bootInfo.PhysicalMemoryOffset);

	SK_LOG_INFO("Initializing the kernel's memory heap");
	result = g_heapMemoryAllocator.Init(102400, VirtualAddress(0x6969'6969'0000));
	if (result.IsError()) {
		SK_LOG_ERROR("Could not initialize the kernel's heap memory pool");
	}

	SK_LOG_INFO("Parsing the ACPI structures");
	result = InitXSDT();
	if (result.IsError()) {
		SK_LOG_ERROR("An unexpected error occured while trying to parse ACPI structures");
	}

	// __asm__ volatile("int3");

	while (true)
		__asm__("hlt");
}
