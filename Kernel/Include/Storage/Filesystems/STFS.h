#pragma once

#include "Core.h"
#include "Result.h"

typedef struct STFSFileListEntry {
	i8 FileName[32];
	usz FileSize;
	u64 FileContentOffset;
} STFSFileListEntry;

typedef struct STFSSuperblock {
	u32 Signature;
	u32 Reserved;
	usz FileCount;
	STFSFileListEntry Files[];
} STFSSuperblock;

Result ReadFileSTFS(const i8* fileName, void* buffer);
