#include "Memory.h"
#include "UefiTypes.h"

VOID MemoryFill(VOID* ptr, UINT8 value, UINTN size)
{
	UINT8* p = ptr;
	while(size > 0)
	{
		p[--size] = value;
	}
}

inline UINT16 VirtualAddressPageOffset(EFI_VIRTUAL_ADDRESS address) {
	return address & VIRTUAL_ADDRESS_PAGE_OFFSET_MASK;
}

inline UINT16 VirtualAddressP1Index(EFI_VIRTUAL_ADDRESS address) {
	return (address >> 12) & VIRTUAL_ADDRESS_ENTRY_INDEX_MASK;
}

inline UINT16 VirtualAddressP2Index(EFI_VIRTUAL_ADDRESS address) {
	return (address >> 21) & VIRTUAL_ADDRESS_ENTRY_INDEX_MASK;
}

inline UINT16 VirtualAddressP3Index(EFI_VIRTUAL_ADDRESS address) {
	return (address >> 30) & VIRTUAL_ADDRESS_ENTRY_INDEX_MASK;
}

inline UINT16 VirtualAddressP4Index(EFI_VIRTUAL_ADDRESS address) {
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

inline UINT64 PageTableEntry(EFI_PHYSICAL_ADDRESS address, UINT16 flags)
{
	return address | flags;
}

EFI_STATUS MapMemoryPage(EFI_VIRTUAL_ADDRESS pageStart, EFI_PHYSICAL_ADDRESS frameStart, EFI_PHYSICAL_ADDRESS p4PhysicalAddress)
{
	// The page and the frame start addresses must all be 4096 (0x1000) aligned,
	// otherwise something went terribly wrong
	if((pageStart & 0xfff) != 0)
		return EFI_INVALID_PARAMETER;

	if((frameStart & 0xfff) != 0)
		return EFI_INVALID_PARAMETER;

	// TODO: This function assumes that all intermediate tables exist,
	// add code to check if they do actually exist and if not create them
	UINT16 p4Index = VirtualAddressP4Index(pageStart);
	EFI_PHYSICAL_ADDRESS p3PhysicalAddress = TableEntryPhysicalAddress(p4PhysicalAddress, p4Index);

	UINT16 p3Index = VirtualAddressP3Index(pageStart);
	EFI_PHYSICAL_ADDRESS p2PhysicalAddress = TableEntryPhysicalAddress(p3PhysicalAddress, p3Index);

	UINT16 p2Index = VirtualAddressP2Index(pageStart);
	EFI_PHYSICAL_ADDRESS p1PhysicalAddress = TableEntryPhysicalAddress(p2PhysicalAddress, p2Index);

	UINT16 p1Index = VirtualAddressP1Index(pageStart);
	UINT64* p1Entries = (UINT64*) p1PhysicalAddress;
	p1Entries[p1Index] = PageTableEntry(frameStart, PRESENT | WRITEABLE);

	return EFI_SUCCESS;
}

