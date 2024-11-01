#include "Core.hpp"

#include "GDT.hpp"
#include "IDT.hpp"
#include "Logger.hpp"
#include "Memory/FrameAllocator.hpp"
#include "PIC.hpp"

#ifndef __x86_64__
#error SaturnKernel requires the x86 64-bit architecture to run properly!
#endif

// Initially empty.
SaturnKernel::KernelBootInfo SaturnKernel::g_bootInfo = {};
SaturnKernel::SequentialFrameAllocator g_frameAllocator = {};

/// C linking so the linker and the bootloader don't absolutely shit themselves
extern "C" auto KernelMain(SaturnKernel::KernelBootInfo* bootInfo) -> void
{
	// Copy the structure provided by the bootloader right at the beginning, so every part of the code can safely access it.
	SaturnKernel::g_bootInfo = *bootInfo;

	// There is a guarantee that the bootloader will set up the framebuffer,
	// so this function doesn't throw but just warns when the serial output device is not available.
	SaturnKernel::g_mainLogger.Init(true, true, SaturnKernel::g_bootInfo, 0x3f8);

	SK_LOG_INFO("Initializing the SaturnOS Kernel\n");

	SK_LOG_INFO("SaturnOS Copyright (C) 2024 StillTree (Alexander Debowski)");
	SK_LOG_INFO("This program comes with ABSOLUTELY NO WARRANTY; for details type ``.");
	SK_LOG_INFO("This is free software, and you are welcome to redistribute it");
	SK_LOG_INFO("under certain conditions; type `` for details.\n");

	SaturnKernel::DisableInterrupts();

	SK_LOG_INFO("Initializing the GDT");
	SaturnKernel::InitGDT();
	SK_LOG_INFO("Initializing the IDT");
	SaturnKernel::InitIDT();

	SK_LOG_INFO("Initializing the Intel PIC 8259");
	SaturnKernel::ReinitializePIC();

	SaturnKernel::EnableInterrupts();

	SK_LOG_INFO("Initializing the sequential frame allocator");
	auto result = g_frameAllocator.Init(
		static_cast<SaturnKernel::MemoryMapEntry*>(SaturnKernel::g_bootInfo.MemoryMap), SaturnKernel::g_bootInfo.MemoryMapEntries);
	if (result.IsError()) {
		SK_LOG_ERROR("Could not initialize the frame allocator");
	}

	SK_LOG_DEBUG("Mapped Physical memory offset: {}", SaturnKernel::g_bootInfo.PhysicalMemoryOffset);

	// __asm__ volatile("int3");

	while (true)
		__asm__("hlt");
}
