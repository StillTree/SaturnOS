#pragma once

#include "Core.h"
#include "Memory/SizedBlockAllocator.h"
#include "Result.h"
#include "Storage/VirtualFileSystem.h"

constexpr usz STFS_MAX_OPENED_INODES = 64;

typedef struct STFSFileListEntry {
	i8 FileName[32];
	usz FileSize;
	u64 FileContentOffset;
	u64 FileID;
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
Result STFSFileInformation(void* fileSystemSpecific, OpenedFileInformation* fileInformation);
Result STFSFileClose(void* fileSystemSpecific);
Result STFSFileLookupID(const i8* fileName, u64* id);

extern STFSDriver g_stfsDriver;
