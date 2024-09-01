#pragma once

#include "UefiTypes.h"

#define PRESENT         ((UINT64) 1)
#define WRITEABLE       ((UINT64) 1 << 1)
#define USER_ACCESSIBLE ((UINT64) 1 << 2)
#define WRITE_THROUGH   ((UINT64) 1 << 3)
#define NO_CACHE        ((UINT64) 1 << 4)
// Set by the CPU, only for checking
#define ACCESSED        ((UINT64) 1 << 5)
#define DIRTY           ((UINT64) 1 << 6)
#define HUGE_PAGE       ((UINT64) 1 << 7)
#define GLOBAL          ((UINT64) 1 << 8)
#define NO_EXECUTE      ((UINT64) 1 << 63)

#define PHYSICAL_ADDRESS_MASK 0x000ffffffffff000

#define PAGE_TABLE_ENTRIES 512

/// C's memset but without a shitty name.
VOID MemoryFill(VOID* ptr, UINT8 value, UINTN size);

EFI_STATUS InitEmptyPageTable(EFI_PHYSICAL_ADDRESS tableAddress);

