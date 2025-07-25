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
	g_idt[vector].Flags = flags | IDTEntryPresent;
	g_idt[vector].Reserved = 0;
}

void InitIDT()
{
	// TODO: The rest of exception handlers
	SetIDTEntry(3, (u64)BreakpointInterruptHandler, IDTEntryInterruptGate | IDTEntryDPL0, 0);
	SetIDTEntry(6, (u64)InvalidOpcodeInterruptHandler, IDTEntryInterruptGate | IDTEntryDPL0, 0);
	SetIDTEntry(8, (u64)DoubleFaultInterruptHandler, IDTEntryInterruptGate | IDTEntryDPL0, 1);
	SetIDTEntry(13, (u64)GeneralProtectionFaultInterruptHandler, IDTEntryInterruptGate | IDTEntryDPL0, 0);
	SetIDTEntry(14, (u64)PageFaultInterruptHandler, IDTEntryInterruptGate | IDTEntryDPL0, 2);

	SetIDTEntry(33, (u64)KeyboardInterruptHandler, IDTEntryTrapGate | IDTEntryDPL0, 0);
	SetIDTEntry(34, (u64)ScheduleInterruptHandler, IDTEntryInterruptGate | IDTEntryDPL0, 7);

	IDTRegister idtRegister = {};
	idtRegister.Size = 0xfff;
	idtRegister.Address = (u64)&g_idt;

	__asm__ volatile("lidt %0\n\t" : : "m"(idtRegister));
}
