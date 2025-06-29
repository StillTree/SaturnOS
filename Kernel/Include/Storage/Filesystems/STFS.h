#pragma once

#include "Core.h"
#include "Memory/SizedBlockAllocator.h"
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

typedef struct STFSDriver {
	SizedBlockAllocator INodeTable;
} STFSDriver;

Result InitSTFS();
Result STFSFileOpen(const i8* fileName, void** fileSystemSpecific);
Result STFSFileRead(void* fileSystemSpecific, usz fileOffset, usz countBytes, void* buffer);
Result STFSFileClose(void* fileSystemSpecific);

extern STFSDriver g_stfsDriver;
