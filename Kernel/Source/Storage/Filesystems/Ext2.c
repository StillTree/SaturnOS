#include "Storage/Filesystems/Ext2.h"

#include "Memory.h"
#include "Memory/BitmapFrameAllocator.h"
#include "Storage/Drivers/AHCI.h"
#include "Storage/GPT.h"

Ext2Driver g_ext2Driver;

static Result GetInode(Ext2Driver* ext2, u32 inode, Frame4KiB inodeTableFrame, Ext2INode** inodePointer)
{
	u32 rootGroup = (inode - 1) / ext2->Superblock->InodesPerGroup;
	u32 tableIndex = (inode - 1) % ext2->Superblock->InodesPerGroup;

	Result result = AHCIDeviceReadSectors(&g_ahciDriver.Devices[0],
		(u32)g_usablePartitions[0].StartLBA + (ext2->BlockGroupDescriptorTable[rootGroup].InodeTableBAddress * 8) + (tableIndex / 2), 1,
		inodeTableFrame);
	if (result) {
		return result;
	}
	Ext2INode* inodeTable = PhysicalAddressAsPointer(inodeTableFrame);

	*inodePointer = inodeTable + (tableIndex % 2);
	return ResultOk;
}

Result GetInodeFromPath(Ext2Driver* ext2, const i8* filePath, Frame4KiB inodeTableFrame, Ext2INode** inodePointer)
{
	if (filePath[0] != '/')
		return ResultInvalidPath;

	// Inclusive
	usz start = 1;
	// Exclusive
	usz end = 1;

	Ext2INode* currentInode = ext2->RootInode;

	Frame4KiB directoryEntriesFrame;
	Result result = AllocateFrame(&g_frameAllocator, &directoryEntriesFrame);
	if (result) {
		return result;
	}

	// Currently, when this algorithm encounters a symlink it will most likely shit itself
	while (filePath[end]) {
		while (filePath[end] == '/')
			end++;

		start = end;

		while (filePath[end] && filePath[end] != '/')
			end++;

		result = AHCIDeviceReadSectors(&g_ahciDriver.Devices[0],
			g_usablePartitions[0].StartLBA + ((usz)currentInode->DirectPointers[0] * 8), 8, directoryEntriesFrame);
		if (result) {
			DeallocateFrame(&g_frameAllocator, directoryEntriesFrame);
			return result;
		}

		// Read directory entries
		u8* inodeEntries = (u8*)PhysicalAddressAsPointer(directoryEntriesFrame);
		Ext2DirectoryEntry* entry = (Ext2DirectoryEntry*)inodeEntries;
		bool found = false;
		while (entry->Inode != 0) {
			// If the length of the name or the name itself differs, continue
			if (end - start != entry->NameLengthLow || !MemoryCompare(filePath + start, entry->Name, end - start)) {
				inodeEntries += entry->Size;
				entry = (Ext2DirectoryEntry*)inodeEntries;
				continue;
			}

			// If we hit a non directory type, but this is not the last path segment, error out, the path is invalid
			if (filePath[end] && entry->Type != DirectoryEntryTypeDirectory) {
				DeallocateFrame(&g_frameAllocator, directoryEntriesFrame);
				return ResultInvalidPath;
			}

			// If the upper conditions are not met, we found the next path segment and it is (hopefully :D) valid
			result = GetInode(ext2, entry->Inode, inodeTableFrame, &currentInode);
			if (result) {
				DeallocateFrame(&g_frameAllocator, directoryEntriesFrame);
				return result;
			}
			found = true;
			break;
		}

		// But if no entries matched, this path doesn't exist
		if (!found) {
			DeallocateFrame(&g_frameAllocator, directoryEntriesFrame);
			return ResultNotFound;
		}
	}

	*inodePointer = currentInode;
	DeallocateFrame(&g_frameAllocator, directoryEntriesFrame);

	return ResultOk;
}

Result InitExt2()
{
	Frame4KiB superblockFrame;
	Result result = AllocateFrame(&g_frameAllocator, &superblockFrame);
	if (result) {
		return result;
	}
	result = AHCIDeviceReadSectors(&g_ahciDriver.Devices[0], g_usablePartitions[0].StartLBA + 2, 2, superblockFrame);
	if (result) {
		return result;
	}
	g_ext2Driver.Superblock = PhysicalAddressAsPointer(superblockFrame);
	g_ext2Driver.BlockSizeBytes = 1024 << g_ext2Driver.Superblock->BlockSize;
	g_ext2Driver.BlockGroupCount
		= (g_ext2Driver.Superblock->InodeCount + g_ext2Driver.Superblock->InodesPerGroup - 1) / g_ext2Driver.Superblock->InodesPerGroup;

	Frame4KiB bgdtFrame;
	result = AllocateFrame(&g_frameAllocator, &bgdtFrame);
	if (result) {
		return result;
	}
	result = AHCIDeviceReadSectors(&g_ahciDriver.Devices[0], g_usablePartitions[0].StartLBA + 8, 2, bgdtFrame);
	if (result) {
		return result;
	}
	g_ext2Driver.BlockGroupDescriptorTable = PhysicalAddressAsPointer(bgdtFrame);

	Frame4KiB rootInodeFrame;
	result = AllocateFrame(&g_frameAllocator, &rootInodeFrame);
	if (result) {
		return result;
	}
	result = GetInode(&g_ext2Driver, 2, rootInodeFrame, &g_ext2Driver.RootInode);
	if (result) {
		return result;
	}

	Frame4KiB rootDirectoryEntriesFrame;
	result = AllocateFrame(&g_frameAllocator, &rootDirectoryEntriesFrame);
	if (result) {
		return result;
	}
	result = AHCIDeviceReadSectors(
		&g_ahciDriver.Devices[0], 2048 + (g_ext2Driver.RootInode->DirectPointers[0] * 8), 8, rootDirectoryEntriesFrame);
	if (result) {
		return result;
	}
	g_ext2Driver.RootInodeEntries = PhysicalAddressAsPointer(rootDirectoryEntriesFrame);

	return ResultOk;
}
