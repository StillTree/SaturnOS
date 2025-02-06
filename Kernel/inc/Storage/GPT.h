#include "Core.h"
#include "Result.h"

typedef struct __attribute__((packed)) GPTHeader {
	i8 Signature[8];
	u32 Revision;
	u32 HeaderSize;
	u32 Checksum;
	u32 Reserved;
	u64 HeaderLBA;
	u64 AlternateHeaderLBA;
	u64 FirstUsableLBA;
	u64 LastUsableLBA;
	GUID GUID;
	u64 PartitionTableLBA;
	u32 PartitionEntryCount;
	u32 PartitionEntrySize;
	u32 PartitionTableCRC32;
} GPTHeader;

typedef struct __attribute__((packed)) GPTEntry {
	GUID TypeGUID;
	GUID PartitionGUID;
	u64 StartLBA;
	u64 EndLBA;
	u64 Attributes;
	u16 Name[36];
} GPTEntry;

Result DetectGPTPartitions();
