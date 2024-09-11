#include "Core.hpp"

#include "IDT.hpp"

#ifndef __x86_64__
#error SaturnKernel requires an x86 64-bit architecture to run properly!
#endif

/// C linking so the linker doesn't absolutely shit itself
extern "C" void KernelMain(SaturnKernel::KernelBootInfo* bootInfo)
{
	InitIDT();

	__asm__ volatile("int3");
	
	U32* framebuffer = reinterpret_cast<U32*>(bootInfo->framebufferAddress);
	for(I32 y = 0; y < 100; y++)
	{
		for(I32 x = 0; x < 100; x++)
		{
			framebuffer[y * bootInfo->framebufferWidth + x] = 0xff8800;
		}
	}

	while(true)
	{
		__asm__("cli; hlt");
	}
}

