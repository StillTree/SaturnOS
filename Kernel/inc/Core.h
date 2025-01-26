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

/// Boot information passed to the kernel by the bootloader.
typedef struct KernelBootInfo {
	u32* Framebuffer;
	u64 FramebufferSize;
	u64 FramebufferWidth;
	u64 FramebufferHeight;
	void* MemoryMap;
	u64 MemoryMapEntries;
	u64 PhysicalMemoryOffset;
	u64 XSDTAddress;
} KernelBootInfo;

/// Globally accessible boot information.
extern KernelBootInfo g_bootInfo;
