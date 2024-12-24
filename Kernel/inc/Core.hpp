#pragma once

#include "STL.hpp"

// Yes, yes, I am aware I could have used the stdint.h header,
// but the kernel only runs on x86, 64-bit, so it doesn't matter if I use these or the variable sized builtins
using I8 = char;
using I16 = short;
using I32 = int;
using I64 = long long;

using U8 = unsigned char;
using U16 = unsigned short;
using U32 = unsigned int;
using U64 = unsigned long long;

using F32 = float;
using F64 = double;

using USIZE = U64;

namespace SaturnKernel {

/// Boot information passed to the kernel by the bootloader.
struct KernelBootInfo {
	U32* Framebuffer;
	U64 FramebufferSize;
	U64 FramebufferWidth;
	U64 FramebufferHeight;
	void* MemoryMap;
	U64 MemoryMapEntries;
	U64 PhysicalMemoryOffset;
};

/// Globally accessible boot information.
extern KernelBootInfo g_bootInfo;

}
