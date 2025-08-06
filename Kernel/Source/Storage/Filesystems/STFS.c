#include "Storage/Filesystems/STFS.h"

#include "Memory.h"
#include "Memory/VirtualMemoryAllocator.h"

STFSDriver g_stfsDriver;

Result InitSTFS()
{
	usz inodeTablePoolSize = Page4KiBNext(sizeof(STFSFileListEntry) * STFS_MAX_OPENED_INODES);

	void* inodeTablePool;
	Result result = AllocateBackedVirtualMemory(&g_kernelMemoryAllocator, inodeTablePoolSize, PageWriteable, &inodeTablePool);
	if (result) {
		return result;
	}

	result = InitSizedBlockAllocator(&g_stfsDriver.INodeTable, inodeTablePool, inodeTablePoolSize, sizeof(STFSFileListEntry));
	if (result) {
		return result;
	}

	return result;
}

Result STFSFileOpen(const i8* fileName, void** fileSystemSpecific)
{
	STFSSuperblock* superblock = g_bootInfo.Ramdisk;

	for (usz i = 0; i < superblock->FileCount; ++i) {
		if (!MemoryCompare(fileName, superblock->Files[i].FileName, StringSize(fileName))) {
			continue;
		}

		STFSFileListEntry* fileListEntry;
		Result result = SizedBlockAllocate(&g_stfsDriver.INodeTable, (void**)&fileListEntry);
		if (result) {
			return result;
		}

		*fileListEntry = superblock->Files[i];
		*fileSystemSpecific = fileListEntry;
		return ResultOk;
	}

	return ResultNotFound;
}

Result STFSFileRead(void* fileSystemSpecific, usz fileOffset, usz countBytes, void* buffer)
{
	if (countBytes <= 0) {
		return ResultOk;
	}

	// TODO: Sanity checks
	STFSSuperblock* superblock = g_bootInfo.Ramdisk;
	STFSFileListEntry* fileListEntry = fileSystemSpecific;

	for (usz i = 0; i < superblock->FileCount; ++i) {
		if (!MemoryCompare(fileListEntry->FileName, superblock->Files[i].FileName, StringSize(fileListEntry->FileName))) {
			continue;
		}

		MemoryCopy((u8*)g_bootInfo.Ramdisk + superblock->Files[i].FileContentOffset + fileOffset, buffer, countBytes);
		return ResultOk;
	}

	return ResultNotFound;
}

Result STFSFileInformation(void* fileSystemSpecific, OpenedFileInformation* fileInformation)
{
	STFSFileListEntry* fileListEntry = fileSystemSpecific;

	fileInformation->Size = fileListEntry->FileSize;
	fileInformation->ID = fileListEntry->FileID;

	return ResultOk;
}

Result STFSFileClose(void* fileSystemSpecific)
{
	STFSSuperblock* superblock = g_bootInfo.Ramdisk;
	STFSFileListEntry* fileListEntry = fileSystemSpecific;

	for (usz i = 0; i < superblock->FileCount; ++i) {
		if (!MemoryCompare(fileListEntry->FileName, superblock->Files[i].FileName, StringSize(fileListEntry->FileName))) {
			continue;
		}

		return SizedBlockDeallocate(&g_stfsDriver.INodeTable, fileSystemSpecific);
	}

	return ResultNotFound;
}

Result STFSFileLookupID(const i8* fileName, u64* id)
{
	STFSSuperblock* superblock = g_bootInfo.Ramdisk;

	for (usz i = 0; i < superblock->FileCount; ++i) {
		if (!MemoryCompare(fileName, superblock->Files[i].FileName, StringSize(fileName))) {
			continue;
		}

		*id = superblock->Files[i].FileID;
		return ResultOk;
	}

	return ResultNotFound;
}
