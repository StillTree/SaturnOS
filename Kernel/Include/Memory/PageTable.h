#pragma once

#include "Core.h"

typedef enum PageTableEntryFlags : u64 {
	PagePresent = 1ULL,
	PageWriteable = 1ULL << 1,
	PageUserAccessible = 1ULL << 2,
	PageWriteThroughCaching = 1ULL << 3,
	PageNoCache = 1ULL << 4,
	PageAccessed = 1ULL << 5,
	PageDirty = 1ULL << 6,
	PageHugePage = 1ULL << 7,
	PageGlobal = 1ULL << 8,
	PageNoExecute = 1ULL << 63,
} PageTableEntryFlags;

typedef u64 PageTableEntry;

constexpr u64 FRAME_ADDRESS_MASK = (((1ULL << 40) - 1) << 12);
constexpr u64 FLAGS_MASK = 0xfff;
constexpr usz PAGE_TABLE_ENTRIES = 512;

void InitEmptyPageTable(PageTableEntry* pageTable);
