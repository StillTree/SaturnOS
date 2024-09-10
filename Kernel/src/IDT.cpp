#include "IDT.hpp"

IDTEntry idt[256];
IDTRegistry idtr;

__attribute__((interrupt)) void InterruptHandler(void* interruptFrame)
{
	__asm__ volatile("outb %b0, %w1" : : "a"('@'), "Nd"(0x3f8) : "memory");
}

void InitIDT()
{
	IDTEntry* entry  = &idt[3];
	entry->ISRLow     = reinterpret_cast<U64>(InterruptHandler) & 0xffff;
	entry->KernelCS   = 0;
	entry->IST        = 0;
	entry->Attributes = 0;
	entry->ISRMiddle  = (reinterpret_cast<U64>(InterruptHandler) >> 16) & 0xffff;
	entry->ISRHigh    = (reinterpret_cast<U64>(InterruptHandler) >> 32) & 0xffffffff;

	idtr.Address = reinterpret_cast<U64>(&idt[0]);
	idtr.Size    = static_cast<U16>(sizeof(IDTEntry) * 4);

	__asm__ volatile("lidt %0" : : "m"(idtr));
	__asm__ volatile("sti");
}

