#pragma once

#include "Core.hpp"

struct __attribute__((packed)) IDTEntry
{
	U64 ISRLow;
	U16 KernelCS;
	U8  IST;
	U8  Attributes;
	U16 ISRMiddle;
	U32 ISRHigh;
	U32 Reserved;
};

struct __attribute__((packed)) IDTRegistry
{
	U16 Size;
	U64 Address;
};

void InitIDT();

