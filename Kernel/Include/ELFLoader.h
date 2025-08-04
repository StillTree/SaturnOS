#pragma once

#include "Core.h"
#include "Result.h"
#include "Scheduler.h"
#include "elf.h"

typedef struct ELFSegmentRegion {
	Page4KiB Begin;
	Page4KiB End;
	PageTableEntryFlags Flags;
	Elf64_Phdr* ELFSegment;
} ELFSegmentRegion;

/// This function should be called only when the interrupt flag is cleared. It can be set afterwards.
Result ProcessLoadELF(Process* process, const i8* elfPath);
