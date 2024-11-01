#pragma once

#include "FrameAllocator.h"
#include "UefiTypes.h"

#define ENTRY_PRESENT ((UINT64)1)
#define ENTRY_WRITEABLE ((UINT64)1 << 1)
#define ENTRY_USER_ACCESSIBLE ((UINT64)1 << 2)
#define ENTRY_WRITE_THROUGH ((UINT64)1 << 3)
#define ENTRY_NO_CACHE ((UINT64)1 << 4)
// Set by the CPU, only for checking
#define ENTRY_ACCESSED ((UINT64)1 << 5)
// Set by the CPU, only for checking
#define ENTRY_DIRTY ((UINT64)1 << 6)
#define ENTRY_HUGE_PAGE ((UINT64)1 << 7)
#define ENTRY_GLOBAL ((UINT64)1 << 8)
#define ENTRY_NO_EXECUTE ((UINT64)1 << 63)

#define PAGE_FRAME_NUMBER_MASK ((1ULL << 40) - 1)
#define PAGE_ENTRY_FLAGS_MASK 0xfff

#define PAGE_TABLE_ENTRIES 512

#define VIRTUAL_ADDRESS_PAGE_OFFSET_MASK 0xfff // Last 12 bits
#define VIRTUAL_ADDRESS_ENTRY_INDEX_MASK ((1UL << 9) - 1) // 9 bits

/// C's memset but without a shitty name.
VOID MemoryFill(VOID* ptr, UINT8 value, UINTN size);
/// C's memcpy byt without a shitty name.
VOID MemoryCopy(VOID* ptr1, VOID* ptr2, UINTN size);
/// C's memcmp but without a shitty name.
INT32 MemoryCompare(const VOID* ptr1, const VOID* ptr2, UINTN size);

/// Returns the starting address for a physical frame that contains the given address
/// (literally just aligns the address to the lower 4096 byte).
EFI_PHYSICAL_ADDRESS PhysFrameContainingAddress(EFI_PHYSICAL_ADDRESS address);

/// Gets the page offset value from the given virtual address.
UINT16 VirtualAddressPageOffset(EFI_VIRTUAL_ADDRESS address);
/// Gets the Level 1 Page Table index from the given virtual address.
UINT16 VirtualAddressP1Index(EFI_VIRTUAL_ADDRESS address);
/// Gets the Level 2 Page Table index from the given virtual address.
UINT16 VirtualAddressP2Index(EFI_VIRTUAL_ADDRESS address);
/// Gets the Level 3 Page Table index from the given virtual address.
UINT16 VirtualAddressP3Index(EFI_VIRTUAL_ADDRESS address);
/// Gets the Level 4 Page Table index from the given virtual address.
UINT16 VirtualAddressP4Index(EFI_VIRTUAL_ADDRESS address);

/// Initializes an empty page table with all entries blank at the given physical address.
EFI_STATUS InitEmptyPageTable(EFI_PHYSICAL_ADDRESS tableAddress);

/// Extracts the physical address from a page table entry,
/// located in the given table's address, at the specified index.
EFI_PHYSICAL_ADDRESS TableEntryPhysicalAddress(EFI_PHYSICAL_ADDRESS tableAddress, UINT16 index);
/// Extracts the page flags from a page table entry,
/// located in the given table's address, at the specified index.
UINT16 TableEntryFlags(EFI_PHYSICAL_ADDRESS tableAddress, UINT16 index);
/// Constructs a page table entry containing the given physical address and flags.
UINT64 PageTableEntry(EFI_PHYSICAL_ADDRESS address, UINT16 flags);

/// Maps the given memory page to the given 4KiB physical memory frame
/// in the given Level 4 Page Table's hierarchy,
/// creating all the necessary intermediate tables if neccessary.
EFI_STATUS MapMemoryPage4KiB(EFI_VIRTUAL_ADDRESS pageStart, EFI_PHYSICAL_ADDRESS frameStart, EFI_PHYSICAL_ADDRESS p4PhysicalAddress,
	FrameAllocatorData* frameAllocator, UINT64 flags);

/// Maps the given memory page to the given 2MiB physical memory frame
/// in the given Level 4 Page Table's hierarchy,
/// creating all the necessary intermediate tables if neccessary.
EFI_STATUS MapMemoryPage2MiB(EFI_VIRTUAL_ADDRESS pageStart, EFI_PHYSICAL_ADDRESS frameStart, EFI_PHYSICAL_ADDRESS p4PhysicalAddress,
	FrameAllocatorData* frameAllocator, UINT64 flags);
