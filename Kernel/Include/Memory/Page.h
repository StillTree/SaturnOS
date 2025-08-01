#pragma once

#include "Core.h"
#include "Memory/Frame.h"
#include "Memory/PageTable.h"
#include "Memory/VirtualAddress.h"
#include "Result.h"

/// Represents a 4 KiB virtual memory page.
typedef u64 Page4KiB;

constexpr u64 PAGE_4KIB_SIZE_BYTES = 4096;

static inline Page4KiB Page4KiBContaining(VirtualAddress address) { return address & ~0xfff; }
static inline Page4KiB Page4KiBNext(VirtualAddress address) { return (address + PAGE_4KIB_SIZE_BYTES - 1) & ~0xfff; }

/// Maps this virtual memory page to the given physical memory frame, using the global frame allocator if needed.
/// Does not flush the TLB.
Result Page4KiBMap(PageTableEntry* p4Table, Page4KiB page, Frame4KiB frame, PageTableEntryFlags flags);
/// Clears the page table entry associated with this page.
/// Does not flush the TLB.
Result Page4KiBUnmap(const PageTableEntry* p4Table, Page4KiB page);
