#include "GDT.hpp"

namespace SaturnKernel
{
	TSS g_tss;
	GDT g_gdt;
	U8 g_doubleFaultStack[20480];
	U8 g_pageFaultStack[20480];

	auto SetGDTEntry64(U64 address, U32 limit, U8 access, U8 flags) -> GDTEntry64
	{
		GDTEntry64 entry	= {};
		entry.AddressLow	= address;
		entry.AddressMiddle = address >> 16;
		entry.AddressHigh	= address >> 24;
		entry.AddressHigher = address >> 32;
		entry.Access		= access;
		entry.Limit			= limit;
		entry.FlagsAndLimit = ((flags & 0xf) << 4) | ((limit >> 16) & 0xf);
		entry.Reserved		= 0;

		return entry;
	}

	auto SetGDTEntry32(U32 address, U32 limit, U8 access, U8 flags) -> GDTEntry32
	{
		GDTEntry32 entry	= {};
		entry.AddressLow	= address;
		entry.AddressMiddle = address >> 16;
		entry.AddressHigh	= address >> 24;
		entry.Access		= access;
		entry.Limit			= limit;
		entry.FlagsAndLimit = ((flags & 0xf) << 4) | ((limit >> 16) & 0xf);

		return entry;
	}

	auto InitGDT() -> void
	{
		g_tss.Reserved1			 = 0;
		g_tss.Reserved2			 = 0;
		g_tss.Reserved3			 = 0;
		g_tss.Reserved4			 = 0;
		g_tss.IOPermissionBitMap = sizeof(g_tss);
		g_tss.IST[0]			 = reinterpret_cast<U64>(g_doubleFaultStack + (sizeof(U8) * 20480));
		g_tss.IST[1]			 = reinterpret_cast<U64>(g_pageFaultStack + (sizeof(U8) * 20480));

		g_gdt.Null		 = SetGDTEntry32(0, 0, 0, 0);
		g_gdt.KernelCode = SetGDTEntry32(0, 0xfffff, 0x9a, 0xa);
		g_gdt.KernelData = SetGDTEntry32(0, 0xfffff, 0x92, 0xc);
		g_gdt.UserCode	 = SetGDTEntry32(0, 0xfffff, 0xfa, 0xa);
		g_gdt.UserData	 = SetGDTEntry32(0, 0xfffff, 0xf2, 0xc);
		g_gdt.TSS		 = SetGDTEntry64(reinterpret_cast<U64>(&g_tss), sizeof(g_tss) - 1, 0x89, 0);

		GDTDescriptor gdtDescriptor = {};
		gdtDescriptor.Address		= reinterpret_cast<U64>(&g_gdt);
		gdtDescriptor.Size			= sizeof(GDT) - 1;

		__asm__ volatile("lgdt %0" : : "m"(gdtDescriptor));
		__asm__ volatile("ltr %0" : : "r"(0x28));

		FlushGDT();
	}
}
