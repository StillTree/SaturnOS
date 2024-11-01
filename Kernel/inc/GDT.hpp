#pragma once

#include "Core.hpp"

namespace SaturnKernel {

struct __attribute__((packed)) GDTDescriptor {
	U16 Size;
	U64 Address;
};

struct __attribute__((packed)) GDTEntry32 {
	U16 Limit;
	U16 AddressLow;
	U8 AddressMiddle;
	U8 Access;
	U8 FlagsAndLimit;
	U8 AddressHigh;
};

struct __attribute__((packed)) GDTEntry64 {
	U16 Limit;
	U16 AddressLow;
	U8 AddressMiddle;
	U8 Access;
	U8 FlagsAndLimit;
	U8 AddressHigh;
	U32 AddressHigher;
	U32 Reserved;
};

struct __attribute__((packed)) GDT {
	GDTEntry32 Null;
	GDTEntry32 KernelCode;
	GDTEntry32 KernelData;
	GDTEntry32 UserCode;
	GDTEntry32 UserData;
	GDTEntry64 TSS;
};

struct __attribute__((packed)) TSS {
	U32 Reserved1;
	U64 RSP[3];
	U64 Reserved2;
	U64 IST[7];
	U64 Reserved3;
	U16 Reserved4;
	U16 IOPermissionBitMap;
};

auto InitGDT() -> void;

}

extern "C" void FlushGDT();
