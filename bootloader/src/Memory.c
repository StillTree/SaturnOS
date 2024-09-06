#include "UefiTypes.h"
#include "Memory.h"
#include "Logger.h"

VOID MemoryFill(VOID* ptr, UINT8 value, UINTN size)
{
	UINT8* p = ptr;
	while(size > 0)
	{
		p[--size] = value;
	}
}

VOID MemoryCopy(VOID* ptr1, VOID* ptr2, UINTN size)
{
	UINT8* src = ptr1; 
	UINT8* dest = ptr2; 

	for(UINTN i = 0; i < size; i++) 
	{
		dest[i] = src[i]; 
	}
}

INT32 MemoryCompare(const VOID* ptr1, const VOID* ptr2, UINTN size)
{
	const UINT8* a = ptr1;
	const UINT8* b = ptr2;

	for(UINTN i = 0; i < size; i++)
	{
		if (a[i] < b[i])
			return -1;
		else if(a[i] > b[i])
			return 1;
	}

	return 0;
}

inline UINT16 VirtualAddressPageOffset(EFI_VIRTUAL_ADDRESS address)
{
	return address & VIRTUAL_ADDRESS_PAGE_OFFSET_MASK;
}

inline UINT16 VirtualAddressP1Index(EFI_VIRTUAL_ADDRESS address)
{
	return (address >> 12) & VIRTUAL_ADDRESS_ENTRY_INDEX_MASK;
}

inline UINT16 VirtualAddressP2Index(EFI_VIRTUAL_ADDRESS address)
{
	return (address >> 21) & VIRTUAL_ADDRESS_ENTRY_INDEX_MASK;
}

inline UINT16 VirtualAddressP3Index(EFI_VIRTUAL_ADDRESS address)
{
	return (address >> 30) & VIRTUAL_ADDRESS_ENTRY_INDEX_MASK;
}

inline UINT16 VirtualAddressP4Index(EFI_VIRTUAL_ADDRESS address)
{
	return (address >> 39) & VIRTUAL_ADDRESS_ENTRY_INDEX_MASK;
}

EFI_STATUS InitEmptyPageTable(EFI_PHYSICAL_ADDRESS tableAddress)
{
	// Check if the address is 4KiB page-aligned, if not we can't create a table there
	if((tableAddress & 0xfff) != 0)
	{
		return EFI_INVALID_PARAMETER;
	}

	UINT64* firstEntry = (UINT64*) tableAddress;

	// An empty table with no entries will just be fully zeroed out
	MemoryFill(firstEntry, 0, 4096);

	return EFI_SUCCESS;
}

EFI_PHYSICAL_ADDRESS TableEntryPhysicalAddress(EFI_PHYSICAL_ADDRESS tableAddress, UINT16 index)
{
	UINT64* table = (UINT64*) tableAddress;
	UINT64 entry = table[index];

	return ((entry >> 12) & PAGE_FRAME_NUMBER_MASK) << 12;
}

UINT16 TableEntryFlags(EFI_PHYSICAL_ADDRESS tableAddress, UINT16 index)
{
	UINT64* table = (UINT64*) tableAddress;
	UINT64 entry = table[index];

	return entry & PAGE_ENTRY_FLAGS_MASK;
}

inline UINT64 PageTableEntry(EFI_PHYSICAL_ADDRESS address, UINT16 flags)
{
	return address | flags;
}

EFI_STATUS MapMemoryPage(
	EFI_VIRTUAL_ADDRESS pageStart,
	EFI_PHYSICAL_ADDRESS frameStart,
	EFI_PHYSICAL_ADDRESS p4PhysicalAddress,
	FrameAllocatorData* frameAllocator)
{
	// The page and the frame start addresses must all be 4096 (0x1000) aligned,
	// otherwise something went terribly wrong
	if((pageStart & 0xfff) != 0)
		return EFI_INVALID_PARAMETER;

	if((frameStart & 0xfff) != 0)
		return EFI_INVALID_PARAMETER;

	UINT16 p4Index = VirtualAddressP4Index(pageStart);
	// If the Level 4 Page Table's entry is not present, create it
	if(!(TableEntryFlags(p4PhysicalAddress, p4Index) & ENTRY_PRESENT))
	{
		EFI_PHYSICAL_ADDRESS newTable = 0;
		EFI_STATUS status = AllocateFrame(frameAllocator, &newTable);
		if(EFI_ERROR(status))
		{
			SN_LOG_ERROR(L"An unexpected error occured while trying to allocate a physical memory frame");
			return status;
		}

		status = InitEmptyPageTable(newTable);
		if(EFI_ERROR(status))
		{
			SN_LOG_ERROR(L"An unexpected error occured while trying to initialize an empty page table");
			return status;
		}

		UINT64* p4Entries = (UINT64*) p4PhysicalAddress;
		p4Entries[p4Index] = PageTableEntry(newTable, ENTRY_PRESENT | ENTRY_WRITEABLE);
	}
	EFI_PHYSICAL_ADDRESS p3PhysicalAddress = TableEntryPhysicalAddress(p4PhysicalAddress, p4Index);

	UINT16 p3Index = VirtualAddressP3Index(pageStart);
	// If the Level 3 Page Table's entry is not present, create it
	if(!(TableEntryFlags(p3PhysicalAddress, p3Index) & ENTRY_PRESENT))
	{
		EFI_PHYSICAL_ADDRESS newTable = 0;
		EFI_STATUS status = AllocateFrame(frameAllocator, &newTable);
		if(EFI_ERROR(status))
		{
			SN_LOG_ERROR(L"An unexpected error occured while trying to allocate a physical memory frame");
			return status;
		}

		status = InitEmptyPageTable(newTable);
		if(EFI_ERROR(status))
		{
			SN_LOG_ERROR(L"An unexpected error occured while trying to initialize an empty page table");
			return status;
		}

		UINT64* p3Entries = (UINT64*) p3PhysicalAddress;
		p3Entries[p3Index] = PageTableEntry(newTable, ENTRY_PRESENT | ENTRY_WRITEABLE);
	}
	EFI_PHYSICAL_ADDRESS p2PhysicalAddress = TableEntryPhysicalAddress(p3PhysicalAddress, p3Index);

	UINT16 p2Index = VirtualAddressP2Index(pageStart);
	// If the Level 2 Page Table's entry is not present, create it
	if(!(TableEntryFlags(p2PhysicalAddress, p2Index) & ENTRY_PRESENT))
	{
		EFI_PHYSICAL_ADDRESS newTable = 0;
		EFI_STATUS status = AllocateFrame(frameAllocator, &newTable);
		if(EFI_ERROR(status))
		{
			SN_LOG_ERROR(L"An unexpected error occured while trying to allocate a physical memory frame");
			return status;
		}

		status = InitEmptyPageTable(newTable);
		if(EFI_ERROR(status))
		{
			SN_LOG_ERROR(L"An unexpected error occured while trying to initialize an empty page table");
			return status;
		}

		UINT64* p2Entries = (UINT64*) p2PhysicalAddress;
		p2Entries[p2Index] = PageTableEntry(newTable, ENTRY_PRESENT | ENTRY_WRITEABLE);
	}
	EFI_PHYSICAL_ADDRESS p1PhysicalAddress = TableEntryPhysicalAddress(p2PhysicalAddress, p2Index);

	UINT16 p1Index = VirtualAddressP1Index(pageStart);
	UINT64* p1Entries = (UINT64*) p1PhysicalAddress;
	// If the given entry is already mapped there is nothing more to do 
	if(TableEntryFlags(p1PhysicalAddress, p1Index) & ENTRY_PRESENT)
	{
		SN_LOG_WARN(L"An attempt was made to map an existing page table entry");
		return EFI_INVALID_PARAMETER;
	}
	p1Entries[p1Index] = PageTableEntry(frameStart, ENTRY_PRESENT | ENTRY_WRITEABLE);

	return EFI_SUCCESS;
}

