#pragma once

#include "UefiTypes.h"

// So I could make this more descriptive by adding regions marked for example
// as kernel executable, runtime services or anything else.
// But this doesn't change shit, the kernel will still boil it all down to
// usable or unusable. So I just won't bother even doing it.
typedef struct MemoryMapEntry
{
	EFI_PHYSICAL_ADDRESS PhysicalStart;
	UINTN                NumberOfPages;
} MemoryMapEntry;

