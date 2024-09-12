#pragma once

#include "Core.hpp"

struct __attribute__((packed)) GDTDescriptor
{
	U16 Size;
	U64 Address;
};

struct __attribute__((packed)) GDTEntry
{
	U16 Limit;
	U16 AddressLow;
	U8  AddressMiddle;
	U8  Access;
	U8  FlagsAndLimit;
	U8  AddressHigh;
};

void InitGDT();

