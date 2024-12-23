#pragma once

#include "Core.hpp"
#include "Memory/VirtualAddress.hpp"

namespace SaturnKernel {

struct Size4KiB { };
struct Size2MiB { };
struct Size1GiB { };

/// C's memset but without a shitty name.
auto MemoryFill(void* ptr, U8 value, USIZE size) -> void;
/// C's memcpy but without a shitty name.
auto MemoryCopy(void* ptr1, void* ptr2, USIZE size) -> void;
/// C's memcmp but without a shitty name.
auto MemoryCompare(const void* ptr1, const void* ptr2, USIZE size) -> I32;

/// Invalidates the whole TLB cache by reloading the CR3 register.
inline auto FlushTLB() -> void
{
	U64 pml4Address = 0;
	__asm__ volatile("mov %%cr3, %0" : "=r"(pml4Address));
	__asm__ volatile("mov %0, %%cr3" : : "r"(pml4Address) : "memory");
}

/// Invalidates a memory page which contains the provided virtual address by using the `invlpg` instruction.
inline auto FlushPage(VirtualAddress address) -> void { __asm__ volatile("invlpg (%0)" : : "r"(address.Value) : "memory"); }

}
