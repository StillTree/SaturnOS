#pragma once

#include "Core.h"
#include "Memory/PageTable.h"
#include "Memory/PhysAddr.h"
#include "Result.h"

typedef u64 VirtAddr;

constexpr u64 INDEX_MASK = ((1ULL << 9) - 1);
constexpr u64 PAGE_OFFSET_MASK = 0xfff;

Result VirtAddrToPhys(const PageTableEntry* p4Table, VirtAddr address, PhysAddr* physAddr);

static inline u16 VirtAddrPageOffset(VirtAddr address) { return address & PAGE_OFFSET_MASK; }

static inline u16 VirtAddrPage1Index(VirtAddr address) { return (address >> 12) & INDEX_MASK; }

static inline u16 VirtAddrPage2Index(VirtAddr address) { return (address >> 21) & INDEX_MASK; }

static inline u16 VirtAddrPage3Index(VirtAddr address) { return (address >> 30) & INDEX_MASK; }

static inline u16 VirtAddrPage4Index(VirtAddr address) { return (address >> 39) & INDEX_MASK; }
