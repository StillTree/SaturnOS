#include "Core.hpp"

#include "FrameAllocator.hpp"
#include "GDT.hpp"
#include "IDT.hpp"
#include "Logger.hpp"
#include "PIC.hpp"

#ifndef __x86_64__
	#error SaturnKernel requires the x86 64-bit architecture to run properly!
#endif

// Initially empty.
SaturnKernel::KernelBootInfo SaturnKernel::g_bootInfo	= {};
SaturnKernel::SequentialFrameAllocator g_frameAllocator = {};

/// C linking so the linker and the bootloader don't absolutely shit themselves
extern "C" void KernelMain(SaturnKernel::KernelBootInfo* bootInfo)
{
	// Copy the structure provided by the bootloader right at the beginning, so every part of the code can safely access it.
	SaturnKernel::g_bootInfo = *bootInfo;

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

	__asm__ volatile("int3");

	g_frameAllocator.Init(
		reinterpret_cast<SaturnKernel::MemoryMapEntry*>(SaturnKernel::g_bootInfo.memoryMapAddress),
		SaturnKernel::g_bootInfo.memoryMapEntries);

	while(true)
		__asm__("hlt");
}
