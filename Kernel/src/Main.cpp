#include "Core.hpp"

#include "GDT.hpp"
#include "IDT.hpp"
#include "Logger.hpp"

#ifndef __x86_64__
	#error SaturnKernel requires the x86 64-bit architecture to run properly!
#endif

// Initially empty.
SaturnKernel::KernelBootInfo SaturnKernel::g_bootInfo = {};

/// C linking so the linker and the bootloader don't absolutely shit themselves
extern "C" void KernelMain(SaturnKernel::KernelBootInfo* bootInfo)
{
	// Copy the structure provided by the bootloader right at the beginning, so every part of the code can safely access it.
	SaturnKernel::g_bootInfo = *bootInfo;

	SaturnKernel::g_mainLogger.Init(true, true, SaturnKernel::g_bootInfo, 0x3f8);
	SK_LOG_INFO("Initializing the SaturnOS Kernel");

	__asm__("cli");

	SK_LOG_INFO("Initializing the GDT");
	SaturnKernel::InitGDT();
	SK_LOG_INFO("Initializing the IDT");
	SaturnKernel::InitIDT();

	__asm__ volatile("int3");

	while(true)
	{
		__asm__("cli; hlt");
	}
}
