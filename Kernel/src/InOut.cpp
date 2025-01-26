#include "InOut.hpp"

namespace SaturnKernel {

void OutputU8(u16 port, u8 value) { __asm__ volatile("outb %b0, %w1" : : "a"(value), "Nd"(port) : "memory"); }

auto InputU8(u16 port) -> u8
{
	u8 result = -1;
	__asm__ volatile("inb %w1, %b0" : "=a"(result) : "Nd"(port) : "memory");

	return result;
}

void IOWait() { OutputU8(0x80, 0); }

}
