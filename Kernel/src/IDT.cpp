#include "IDT.hpp"

#include "InterruptHandlers.hpp"

namespace SaturnKernel
{
	IDTEntry g_idt[256];

	void SetIDTEntry(U8 vector, U64 handlerFn)
	{
		IDTEntry& entry   = g_idt[vector];
		entry.AddressLow  = handlerFn & 0xffff;
		entry.AddressMid  = (handlerFn >> 16) & 0xffff;
		entry.AddressHigh = handlerFn >> 32;
		entry.KernelCS    = 0x8;
		entry.Flags       = 0x8e;
		entry.IST         = 0;
		entry.Reserved    = 0;
	}

	void InitIDT()
	{
		for(int i = 0; i < 5; i++)
		{
			SetIDTEntry(i, reinterpret_cast<U64>(SaturnKernel::test));
		}

		IDTRegister idtRegister;
		idtRegister.Size    = 0xfff;
		idtRegister.Address = reinterpret_cast<U64>(&g_idt);

		__asm__ volatile(
			"lidt %0\n\t"
			"sti\n\t"
			:
			: "m"(idtRegister));
	}
}

