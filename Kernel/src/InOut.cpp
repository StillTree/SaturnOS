#include "InOut.hpp"

namespace SaturnKernel
{
	void OutputU8(U16 port, U8 value)
	{
		__asm__ volatile("outb %b0, %w1" : : "a"(value), "Nd"(port) : "memory");
	}

	U8 InputU8(U16 port)
	{
		U8 result;
		__asm__ volatile("inb %w1, %b0" : "=a"(result) : "Nd"(port) : "memory");

		return result;
	}
}

