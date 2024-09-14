#include "InterruptHandlers.hpp"

namespace SaturnKernel
{
	__attribute__((interrupt)) void BreakpointInterruptHandler(InterruptFrame* frame)
	{
		__asm__ volatile("outb %b0, %w1" : : "a"('b'), "Nd"(0x3f8) : "memory");
	}

	__attribute__((interrupt)) void DoubleFaultInterruptHandler(InterruptFrame* frame, U64)
	{
		__asm__ volatile("outb %b0, %w1" : : "a"('d'), "Nd"(0x3f8) : "memory");

		while(true)
			__asm__ volatile("cli; hlt");
	}

	__attribute__((interrupt)) void PageFaultInterruptHandler(InterruptFrame* frame, U64 errorCode)
	{
		U64 faultVirtualAddress;
		__asm__ volatile("mov %%cr2, %0" : "=r"(faultVirtualAddress));

		U64 pml4Address;
		__asm__ volatile("mov %%cr3, %0" : "=r"(pml4Address));

		// TODO: Using the errorCode figure the rest of the shit out

		__asm__ volatile("outb %b0, %w1" : : "a"('p'), "Nd"(0x3f8) : "memory");

		while(true)
			__asm__ volatile("cli; hlt");
	}
}

