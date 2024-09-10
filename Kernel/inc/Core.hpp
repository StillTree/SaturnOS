#pragma once

// Yes, yes, I am aware I could have used the stdint.h header,
// but it runs only on x86 64-bit so it doesn't matter if I use them or the variable sized builtins
typedef signed char      I8;
typedef signed short     I16;
typedef signed int       I32;
typedef signed long long I64;

typedef unsigned char      U8;
typedef unsigned short     U16;
typedef unsigned int       U32;
typedef unsigned long long U64;

typedef float  F32;
typedef double F64;

typedef U64 USIZE;

namespace SaturnKernel
{
	/// Boot information passed to the kernel by the bootloader.
	struct KernelBootInfo
	{
		U64 framebufferAddress;
		U64 framebufferSize;
		U64 framebufferWidth;
		U64 framebufferHeight;
	};
}

