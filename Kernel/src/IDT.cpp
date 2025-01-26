#include "IDT.hpp"

#include "InterruptHandlers.hpp"

namespace SaturnKernel {

IDTEntry g_idt[256];

auto SetIDTEntry(u8 vector, u64 handlerFn, u8 flags, u8 istNumber) -> void
{
	IDTEntry& entry = g_idt[vector];
	entry.AddressLow = handlerFn;
	entry.AddressMid = handlerFn >> 16;
	entry.AddressHigh = handlerFn >> 32;
	entry.KernelCS = 0x8;
	entry.IST = istNumber;
	entry.Flags = flags;
	entry.Reserved = 0;
}

auto InitIDT() -> void
{
	// TODO: The rest of exception handlers
	SetIDTEntry(3, reinterpret_cast<u64>(BreakpointInterruptHandler), 0x8e, 0);
	SetIDTEntry(8, reinterpret_cast<u64>(DoubleFaultInterruptHandler), 0x8f, 1);
	SetIDTEntry(14, reinterpret_cast<u64>(PageFaultInterruptHandler), 0x8f, 2);

	SetIDTEntry(33, reinterpret_cast<u64>(KeyboardInterruptHandler), 0x8e, 0);
	SetIDTEntry(34, reinterpret_cast<u64>(TestInterruptHandler), 0x8e, 0);

	IDTRegister idtRegister = {};
	idtRegister.Size = 0xfff;
	idtRegister.Address = reinterpret_cast<u64>(&g_idt);

	__asm__ volatile("lidt %0\n\t" : : "m"(idtRegister));
}

}
