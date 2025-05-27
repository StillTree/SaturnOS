#include "GDT.h"

#include "Core.h"

TSS g_tss;
GDT g_gdt;
u8 g_doubleFaultStack[20480];
u8 g_pageFaultStack[20480];
u8 g_schedulerInterruptStack[20480];

GDTEntry64 SetGDTEntry64(u64 address, u32 limit, u8 access, u8 flags)
{
	GDTEntry64 entry = {};
	entry.AddressLow = address;
	entry.AddressMiddle = address >> 16;
	entry.AddressHigh = address >> 24;
	entry.AddressHigher = address >> 32;
	entry.Access = access;
	entry.Limit = limit;
	entry.FlagsAndLimit = ((flags & 0xf) << 4) | ((limit >> 16) & 0xf);
	entry.Reserved = 0;

	return entry;
}

GDTEntry32 SetGDTEntry32(u32 address, u32 limit, u8 access, u8 flags)
{
	GDTEntry32 entry = {};
	entry.AddressLow = address;
	entry.AddressMiddle = address >> 16;
	entry.AddressHigh = address >> 24;
	entry.Access = access;
	entry.Limit = limit;
	entry.FlagsAndLimit = ((flags & 0xf) << 4) | ((limit >> 16) & 0xf);

	return entry;
}

void InitGDT()
{
	g_tss.Reserved1 = 0;
	g_tss.Reserved2 = 0;
	g_tss.Reserved3 = 0;
	g_tss.Reserved4 = 0;
	g_tss.IOPermissionBitMap = sizeof(g_tss);
	g_tss.IST[0] = (u64)g_doubleFaultStack + (sizeof(u8) * 20480);
	g_tss.IST[1] = (u64)g_pageFaultStack + (sizeof(u8) * 20480);
	g_tss.IST[6] = (u64)g_schedulerInterruptStack + (sizeof(u8) * 20480);
	g_tss.RSP[0] = g_bootInfo.KernelStackTop;

	g_gdt.Null = SetGDTEntry32(0, 0, 0, 0);
	g_gdt.KernelCode = SetGDTEntry32(0, 0xfffff, 0x9a, 0xa);
	g_gdt.KernelData = SetGDTEntry32(0, 0xfffff, 0x92, 0xc);
	g_gdt.UserData = SetGDTEntry32(0, 0xfffff, 0xf2, 0xc);
	g_gdt.UserCode = SetGDTEntry32(0, 0xfffff, 0xfa, 0xa);
	g_gdt.TSS = SetGDTEntry64((u64)&g_tss, sizeof(g_tss) - 1, 0x89, 0);

	GDTDescriptor gdtDescriptor = {};
	gdtDescriptor.Address = (u64)&g_gdt;
	gdtDescriptor.Size = sizeof(GDT) - 1;

	__asm__ volatile("lgdt %0" : : "m"(gdtDescriptor));
	__asm__ volatile("ltr %0" : : "r"(0x28));

	FlushGDT();
}
