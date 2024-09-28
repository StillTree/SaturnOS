#pragma once

#include "Core.hpp"

namespace SaturnKernel
{
	struct __attribute__((packed)) IDTEntry
	{
		U16 AddressLow;
		U16 KernelCS;
		U8 IST;
		U8 Flags;
		U16 AddressMid;
		U32 AddressHigh;
		U32 Reserved;
	};

	struct __attribute__((packed)) IDTRegister
	{
		U16 Size;
		U64 Address;
	};

	void LoadIDT(void* address);
	void SetIDTEntry(U8 vector, U64 handler);
	void InitIDT();

	extern IDTEntry g_idt[256];
}
