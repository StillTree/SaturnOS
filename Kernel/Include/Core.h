#pragma once

// Yes, yes, I am aware I could have used the stdint.h header,
// but the kernel only runs on x86, 64-bit, so it doesn't matter if I use these or the variable sized builtins
typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef float f32;
typedef double f64;

typedef u64 usz;

constexpr i8 I8_MAX = 0x7f;
constexpr i16 I16_MAX = 0x7fff;
constexpr i32 I32_MAX = 0x7fffffff;
constexpr i64 I64_MAX = 0x7fffffffffffffff;

constexpr u8 U8_MAX = 0xff;
constexpr u16 U16_MAX = 0xffff;
constexpr u32 U32_MAX = 0xffffffff;
constexpr u64 U64_MAX = 0xffffffffffffffff;

constexpr usz USZ_MAX = U64_MAX;

/// Globally Unique Identifier, as defined by th UEFI Specification.
typedef struct GUID {
	u32 Data1;
	u16 Data2;
	u16 Data3;
	u8 Data4[8];
} GUID;

static inline bool GUIDEmpty(GUID guid)
{
	return guid.Data1 == 0 && guid.Data2 == 0 && guid.Data3 == 0 && guid.Data4[0] == 0 && guid.Data4[1] == 0 && guid.Data4[2] == 0
		&& guid.Data4[3] == 0 && guid.Data4[4] == 0 && guid.Data4[5] == 0 && guid.Data4[6] == 0 && guid.Data4[7] == 0;
}

/// Boot information passed to the kernel by the bootloader.
typedef struct BootInfo {
	u32* Framebuffer;
	usz FramebufferSize;
	usz FramebufferWidth;
	usz FramebufferHeight;
	void* MemoryMap;
	u64 MemoryMapEntries;
	u64 PhysicalMemoryOffset;
	u64 PhysicalMemoryMappingSize;
	u64 XSDTPhysicalAddress;
	/// The beginning of the kernel's stack.
	u64 KernelStackTop;
	u64 KernelAddress;
	usz KernelSize;
	u64 ContextSwitchFunctionPage;
	void* Ramdisk;
	usz RamdiskSizeBytes;
	const i8* Args;
	u64 KernelPML4;
} BootInfo;

/// Globally accessible boot information.
extern BootInfo g_bootInfo;
