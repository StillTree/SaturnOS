#pragma once

// Yes, yes, I am aware I could have used the stdint.h header,
// but the kernel only runs on x86, 64-bit, so it doesn't matter if I use these or the variable sized builtins
using i8 = char;
using i16 = short;
using i32 = int;
using i64 = long long;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;

using f32 = float;
using f64 = double;

using usize = u64;

namespace SaturnKernel {

/// Boot information passed to the kernel by the bootloader.
struct KernelBootInfo {
	u32* Framebuffer;
	u64 FramebufferSize;
	u64 FramebufferWidth;
	u64 FramebufferHeight;
	void* MemoryMap;
	u64 MemoryMapEntries;
	u64 PhysicalMemoryOffset;
	u64 XSDTAddress;
};

/// Globally accessible boot information.
extern KernelBootInfo g_bootInfo;

}
