#include "Storage/GPT.h"

#include "Logger.h"
#include "Memory.h"
#include "Memory/BitmapFrameAllocator.h"
#include "Memory/Frame.h"
#include "Memory/PhysicalAddress.h"
#include "Storage/Drivers/AHCI.h"

Partition g_usablePartitions[1];

Result DetectGPTPartitions()
{
	// For now I just assume that the first partition is usable :D
	// Like with everything I will later implement support for multiple partitions or whatever
	Frame4KiB partitionTableHeader = AllocateFrame(&g_frameAllocator);
	Result result = AHCIDeviceReadSectors(&g_ahciDriver.Devices[0], 1, 1, partitionTableHeader);
	if (result) {
		return result;
	}

	GPTHeader* gptHeader = PhysicalAddressAsPointer(partitionTableHeader);

	if (!MemoryCompare(gptHeader->Signature, "EFI PART", 8)) {
		return ResultSerialOutputUnavailable;
	}

	usz neededSectors = ((usz)gptHeader->PartitionEntryCount * gptHeader->PartitionEntrySize + g_ahciDriver.Devices[0].SectorSize - 1)
		/ g_ahciDriver.Devices[0].SectorSize;
	usz neededFrames = (neededSectors + 7) / 8;

	Frame4KiB tableStartFrame;
	result = AllocateContiguousFrames(&g_frameAllocator, neededFrames, &tableStartFrame);
	if (result) {
		return result;
	}

	result = AHCIDeviceReadSectors(&g_ahciDriver.Devices[0], gptHeader->PartitionTableLBA, neededSectors, tableStartFrame);
	if (result) {
		return result;
	}

	GPTEntry* gptEntries = PhysicalAddressAsPointer(tableStartFrame);
	for (u32 i = 0; i < gptHeader->PartitionEntryCount; i++) {
		if (GUIDEmpty(gptEntries[i].PartitionGUID))
			continue;

		LogLine(SK_LOG_DEBUG "Detected usable GPT partition: StartLBA = %u, EndLBA = %u, Attributes = %x, Name = \"%w\", GUID = %g",
			gptEntries[i].StartLBA, gptEntries[i].EndLBA, gptEntries[i].Attributes, gptEntries[i].Name, gptEntries[i].PartitionGUID);
	}

	g_usablePartitions[0].StartLBA = gptEntries[0].StartLBA;
	g_usablePartitions[0].EndLBA = gptEntries[0].EndLBA;

	result = DeallocateContiguousFrames(&g_frameAllocator, tableStartFrame, neededFrames);
	if (result) {
		return result;
	}

	DeallocateFrame(&g_frameAllocator, partitionTableHeader);

	return result;
}
