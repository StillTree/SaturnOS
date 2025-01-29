#include "IDT.h"

#include "InterruptHandlers.h"

IDTEntry g_idt[256];

void SetIDTEntry(u8 vector, u64 handlerFn, u8 flags, u8 istNumber)
{
	g_idt[vector].AddressLow = handlerFn;
	g_idt[vector].AddressMid = handlerFn >> 16;
	g_idt[vector].AddressHigh = handlerFn >> 32;
	g_idt[vector].KernelCS = 0x8;
	g_idt[vector].IST = istNumber;
	g_idt[vector].Flags = flags;
	g_idt[vector].Reserved = 0;
}

void InitIDT()
{
	// TODO: The rest of exception handlers
	SetIDTEntry(3, (u64)BreakpointInterruptHandler, 0x8e, 0);
	SetIDTEntry(8, (u64)DoubleFaultInterruptHandler, 0x8f, 1);
	SetIDTEntry(13, (u64)GeneralProtectionFaultInterruptHandler, 0x8f, 1);
	SetIDTEntry(14, (u64)PageFaultInterruptHandler, 0x8f, 0);

	SetIDTEntry(33, (u64)KeyboardInterruptHandler, 0x8e, 0);

	IDTRegister idtRegister = {};
	idtRegister.Size = 0xfff;
	idtRegister.Address = (u64)&g_idt;

	__asm__ volatile("lidt %0\n\t" : : "m"(idtRegister));
}
