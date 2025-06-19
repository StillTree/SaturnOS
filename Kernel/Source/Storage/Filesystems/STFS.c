#include "Storage/Filesystems/STFS.h"

#include "Memory.h"

Result ReadFileSTFS(const i8* fileName, void* buffer)
{
	STFSSuperblock* superblock = g_bootInfo.Ramdisk;

	for (usz i = 0; i < superblock->FileCount; ++i) {
		if (!MemoryCompare(fileName, superblock->Files[i].FileName, StringSize(fileName))) {
			continue;
		}

		MemoryCopy((u8*)g_bootInfo.Ramdisk + superblock->Files[i].FileContentOffset, buffer, superblock->Files[i].FileSize);
		return ResultOk;
	}

	return ResultNotFound;
}
