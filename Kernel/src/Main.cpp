#include "Core.hpp"

#include "GDT.hpp"
#include "IDT.hpp"
#include "Logger.hpp"

#ifndef __x86_64__
#error SaturnKernel requires an x86 64-bit architecture to run properly!
#endif

/// C linking so the linker and the bootloader don't absolutely shit themselves
extern "C" void KernelMain(SaturnKernel::KernelBootInfo* bootInfo)
{
	SaturnKernel::g_mainLogger.Init(true, true, bootInfo, 0x3f8);
	SK_LOG_INFO("Initializing the SaturnOS Kernel");

	__asm__("cli");

	SK_LOG_INFO("Initializing the GDT");
	SaturnKernel::InitGDT();
	SK_LOG_INFO("Initializing the IDT");
	SaturnKernel::InitIDT();

	//__asm__ volatile("int3");

	SaturnKernel::MemoryMapEntry* entry = reinterpret_cast<SaturnKernel::MemoryMapEntry*>(bootInfo->memoryMapAddress);
	for(USIZE i = 0; i < bootInfo->memoryMapEntries; i++)
	{
		SK_LOG_INFO("Memory map entry: physicalStart = {}, physicalEnd = {}", entry[i].physicalStart, entry[i].physicalEnd);
	}

	while(true)
	{
		__asm__("cli; hlt");
	}
}

