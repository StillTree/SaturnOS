#include "GDT.hpp"

namespace SaturnKernel
{
	GDT g_gdt;

	GDTEntry SetGDTEntry(U32 address, U32 limit, U8 access, U8 flags)
	{
		GDTEntry entry;
		entry.AddressLow    = address;
		entry.AddressMiddle = address >> 16;
		entry.AddressHigh   = address >> 24;
		entry.Access        = access;
		entry.Limit         = limit;
		entry.FlagsAndLimit = ((flags & 0xf) << 4) | ((limit >> 16) & 0xf);

		return entry;
	}

	void InitGDT()
	{
		g_gdt.Null       = SetGDTEntry(0, 0,       0,    0);
		g_gdt.KernelCode = SetGDTEntry(0, 0xfffff, 0x9a, 0xa);
		g_gdt.KernelData = SetGDTEntry(0, 0xfffff, 0x92, 0xc);
		g_gdt.UserCode   = SetGDTEntry(0, 0xfffff, 0xfa, 0xa);
		g_gdt.UserData   = SetGDTEntry(0, 0xfffff, 0xf2, 0xc);

		GDTDescriptor gdtDescriptor;
		gdtDescriptor.Address = reinterpret_cast<U64>(&g_gdt);
		gdtDescriptor.Size    = sizeof(GDT) - 1;

		__asm__ volatile("lgdt %0" : : "m"(gdtDescriptor));

		LoadGDT();
	}
}

