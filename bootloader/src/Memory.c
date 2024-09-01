#include "Memory.h"

VOID MemoryFill(VOID* ptr, UINT8 value, UINTN size)
{
	UINT8* p = ptr;
	while(size > 0)
	{
		p[--size] = value;
	}
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

