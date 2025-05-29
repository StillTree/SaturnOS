#pragma once

#include "Core.h"
#include "Memory/Frame.h"
#include "Result.h"

typedef struct Superblock {
	u32 InodeCount;
	u32 BlockCount;
	u32 SuperuserReservedBlockCount;
	u32 UnallocatedBlockCount;
	u32 UnallocatedInodeCount;
	u32 SuperblockBlockNumber;
	u32 BlockSize;
	u32 FragmentSize;
	u32 BlocksPerGroup;
	u32 FragmentsPerGroup;
	u32 InodesPerGroup;
	u32 LastMountTime;
	u32 LastWriteTime;
	u16 MountCountSinceConsistencyCheck;
	u16 MountsAllowedBeforeConsistencyCheck;
	u16 Ext2Signature;
	u16 FilesystemState;
	u16 ErrorAction;
	u16 VersionMinor;
	u32 LastConsistencyCheckTime;
	u32 IntervalBetweenConsistencyChecks;
	u32 OSCreatedID;
	u32 VersionMajor;
	u16 UserIDReservedBlocks;
	u16 GroupIDReservedBlocks;
	u32 FirstUnreservedInodeNumber;
	u16 InodeSizeBytes;
	u16 BackupThisSuperblockBlockNumber;
	u32 OptionalFeatures;
	u32 RequiredFeatures;
	u32 ReadonlyWhenUnsupportedFeatures;
	u32 FilesystemID[4];
	i8 VolumeName[16];
	i8 LastMountPath[64];
	u32 CompressionAlgorithmsUsed;
	u8 FilePreallocationBlockCount;
	u8 DirectoryPreallocationBlockCount;
	u16 Reserved1;
	u32 JournalID[4];
	u32 JournalInode;
	u32 JoutnalDevice;
	u32 OrphanInodeListHead;
	u8 Reserved2[787];
} Superblock;

typedef struct BlockGroupDescriptor {
	u32 BlockUsageBitmapBAddress;
	u32 InodeUsageBitmapBAddress;
	u32 InodeTableBAddress;
	u16 UnallocatedBlockCount;
	u16 UnallocatedInodeCount;
	u16 DirectoryCount;
	u8 Reserved[12];
} BlockGroupDescriptor;

typedef enum InodeTypeAndPermissions : u16 {
	InodePermissionOtherExecute = 0x1,
	InodePermissionOtherWrite = 0x2,
	InodePermissionOtherRead = 0x4,
	InodePermissionGroupExecute = 0x8,
	InodePermissionGroupWrite = 0x10,
	InodePermissionGroupRead = 0x20,
	InodePermissionUserExecute = 0x40,
	InodePermissionUserWrite = 0x80,
	InodePermissionUserRead = 0x100,
	InodeStickyBit = 0x200,
	InodeGroupIDSet = 0x400,
	InodeUserIDSet = 0x800,
	InodeTypeFIFO = 0x1000,
	InodeTypeCharactedDevice = 0x2000,
	InodeTypeDirectory = 0x4000,
	InodeTypeBlockDevice = 0x6000,
	InodeTypeRegularFile = 0x8000,
	InodeTypeSymlink = 0xa000,
	InodeTypeUnixSocket = 0xc000,
} InodeTypeAndPermissions;

typedef struct Inode {
	InodeTypeAndPermissions TypeAndPermissions;
	u16 UserID;
	u32 SizeBytesLow;
	u32 LastAccessTime;
	u32 CreationTime;
	u32 LastModifyTime;
	u32 DeletionTime;
	u16 GroupID;
	u16 HardLinkCount;
	u32 DiskSectorCount;
	u32 Flags;
	u32 OSSpecific1;
	u32 DirectPointers[12];
	u32 SinglyIndirectPointer;
	u32 DoublyIndirectPointer;
	u32 TriplyIndirectPointer;
	u32 GenerationNumber;
	u32 ExtendedAttributeBlock;
	u32 SizeBytesHigh;
	u32 FragmentBlockAddress;
	u8 OSSpecific2[12];
	u8 Reserved[128];
} Inode;

typedef enum DirectoryEntryType : u8 {
	DirectoryEntryTypeUnknown,
	DirectoryEntryTypeRegularFile,
	DirectoryEntryTypeDirectory,
	DirectoryEntryTypeCharactedDevice,
	DirectoryEntryTypeBlockDevice,
	DirectoryEntryTypeFIFO,
	DirectoryEntryTypeSocket,
	DirectoryEntryTypeSymlink
} DirectoryEntryType;

typedef struct DirectoryEntry {
	u32 Inode;
	u16 Size;
	u8 NameLengthLow;
	DirectoryEntryType Type;
	i8 Name[];
} DirectoryEntry;

typedef struct Ext2Driver {
	u32 BlockGroupCount;
	u32 BlockSizeBytes;

	Superblock* Superblock;
	BlockGroupDescriptor* BlockGroupDescriptorTable;

	Inode* RootInode;
	DirectoryEntry* RootInodeEntries;
} Ext2Driver;

Result GetInodeFromPath(Ext2Driver* ext2, const i8* filePath, Frame4KiB inodeTableFrame, Inode** inodePointer);

Result InitExt2();

extern Ext2Driver g_ext2Driver;
