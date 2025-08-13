#pragma once

#include "Core.h"

constexpr u16 GDT_ENTRY_KERNEL_CODE = 0x8;
constexpr u16 GDT_ENTRY_KERNEL_DATA = 0x10;
constexpr u16 GDT_ENTRY_USER_CODE = 0x23;
constexpr u16 GDT_ENTRY_USER_DATA = 0x1b;

typedef struct __attribute__((packed)) GDTDescriptor {
	u16 Size;
	u64 Address;
} GDTDescriptor;

typedef struct __attribute__((packed)) GDTEntry32 {
	u16 Limit;
	u16 AddressLow;
	u8 AddressMiddle;
	u8 Access;
	u8 FlagsAndLimit;
	u8 AddressHigh;
} GDTEntry32;

typedef struct __attribute__((packed)) GDTEntry64 {
	u16 Limit;
	u16 AddressLow;
	u8 AddressMiddle;
	u8 Access;
	u8 FlagsAndLimit;
	u8 AddressHigh;
	u32 AddressHigher;
	u32 Reserved;
} GDTEntry64;

typedef struct __attribute__((packed)) GDT {
	GDTEntry32 Null;
	GDTEntry32 KernelCode;
	GDTEntry32 KernelData;
	GDTEntry32 UserData;
	GDTEntry32 UserCode;
	GDTEntry64 TSS;
} GDT;

typedef struct __attribute__((packed)) TSS {
	u32 Reserved1;
	u64 RSP[3];
	u64 Reserved2;
	u64 IST[7];
	u64 Reserved3;
	u16 Reserved4;
	u16 IOPermissionBitMap;
} TSS;

void InitGDT();
void FlushGDT();

extern TSS g_tss;
