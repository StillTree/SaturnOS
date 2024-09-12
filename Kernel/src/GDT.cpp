#include "GDT.hpp"

GDTEntry gdt[5];
GDTDescriptor gdtr;

void SetGDTEntry(USIZE index, U32 address, U32 limit, U8 access, U8 flags)
{
	GDTEntry* entry     = &gdt[index];
	entry->AddressLow    = address;
	entry->AddressMiddle = address >> 16;
	entry->AddressHigh   = address >> 24;
	entry->Access        = access;
	entry->Limit         = limit;
	entry->FlagsAndLimit = ((flags & 0xf) << 4) | ((limit >> 16) & 0xf);
}

extern "C" void LoadGDT(GDTDescriptor* gdtr);

void InitGDT()
{
	SetGDTEntry(0, 0, 0,       0,    0);
	SetGDTEntry(1, 0, 0xfffff, 0x9a, 0xa);
	SetGDTEntry(2, 0, 0xfffff, 0x92, 0xc);
	SetGDTEntry(3, 0, 0xfffff, 0xfa, 0xa);
	SetGDTEntry(4, 0, 0xfffff, 0xf2, 0xc);

	gdtr.Address = reinterpret_cast<U64>(&gdt[0]);
	gdtr.Size    = sizeof(GDTEntry) * 5 - 1;

	LoadGDT(&gdtr);
}

