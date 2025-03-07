#pragma once

#include "Core.h"
#include "Memory/PageTable.h"
#include "Memory/PhysicalAddress.h"
#include "Result.h"

typedef u64 VirtualAddress;

constexpr u64 INDEX_MASK = ((1ULL << 9) - 1);
constexpr u64 PAGE_OFFSET_MASK = 0xfff;

Result VirtualAddressToPhysical(VirtualAddress address, const PageTableEntry* p4Table, PhysicalAddress* physicalAddress);

static inline u16 VirtualAddressPageOffset(VirtualAddress address) { return address & PAGE_OFFSET_MASK; }

static inline u16 VirtualAddressPage1Index(VirtualAddress address) { return (address >> 12) & INDEX_MASK; }

static inline u16 VirtualAddressPage2Index(VirtualAddress address) { return (address >> 21) & INDEX_MASK; }

static inline u16 VirtualAddressPage3Index(VirtualAddress address) { return (address >> 30) & INDEX_MASK; }

static inline u16 VirtualAddressPage4Index(VirtualAddress address) { return (address >> 39) & INDEX_MASK; }
