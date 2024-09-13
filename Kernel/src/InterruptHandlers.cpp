#include "InterruptHandlers.hpp"

namespace SaturnKernel
{
	__attribute__((interrupt)) void test(void* zupa)
	{
		__asm__ volatile("outb %b0, %w1" : : "a"('l'), "Nd"(0x3f8) : "memory");
	}
}

