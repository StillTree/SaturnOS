#include "Storage/Drivers/NVMe.h"

#include <stddef.h>

#include "Logger.h"
#include "Memory.h"
#include "Memory/BitmapFrameAllocator.h"
#include "PCI.h"

NVMeDriver g_nvmeDriver;

Result NVMeInit(NVMeDriver* driver)
{
	// For now, just using the first device and ignoring the rest ones
	g_pciStorageDevices[0].ConfigurationSpace->Command |= CommandRegisterEnableBusMaster | CommandRegisterEnableMemory;

	// For now I am not gonna implement MSI-X,
	// When I have a working implementation (with multitasking perhaps) I will do so.
	// TODO: Do that :D
	// Result result = PCIDeviceSetMSIXVector(&g_pciStorageDevices[0], 0, 34);
	// if (result) {
	// 	return result;
	// }
	// PCIDeviceEnableMSIX(&g_pciStorageDevices[0]);

	PhysicalAddress barAddress;
	Result result = PCIDeviceBarAddress(&g_pciStorageDevices[0], 0, &barAddress);
	if (result) {
		return result;
	}
	driver->Registers = (NVMeRegisters*)barAddress;

	driver->Registers->CC = 0;
	while ((driver->Registers->CSTS & 0x1) != 0)
		;

	if ((driver->Registers->CAP & (1UL << 37)) != 0 && (driver->Registers->CAP & (1UL << 43)) == 0) {
		// NCSS
		driver->Registers->CC &= ~(0b111 << 4);
	} else if ((driver->Registers->CAP & (1UL << 43)) != 0) {
		// IOCSS
		driver->Registers->CC |= 0b110 << 4;
	} else if ((driver->Registers->CAP & (1UL << 44)) != 0) {
		// NOIOCSS
		driver->Registers->CC |= 0b111 << 4;
	}

	// Set the Round Robin arbitration mechanism
	driver->Registers->CC &= ~(0xf << 11);
	// Set the memory page size to 4 KiB
	driver->Registers->CC &= ~(0xf << 7);

	Frame4KiB asqFrame;
	result = AllocateFrame(&g_frameAllocator, &asqFrame);
	if (result) {
		return result;
	}
	Frame4KiB acqFrame;
	result = AllocateFrame(&g_frameAllocator, &acqFrame);
	if (result) {
		return result;
	}

	// Just to be sure, zero out all of them
	// After this, the phase tag should be 1 on the first command completion
	MemoryFill(PhysicalAddressAsPointer(asqFrame), 0, FRAME_4KIB_SIZE_BYTES);
	MemoryFill(PhysicalAddressAsPointer(acqFrame), 0, FRAME_4KIB_SIZE_BYTES);

	// Initialize the admin queues
	driver->Registers->ASQ = asqFrame;
	driver->Queues[0].SubmissionQueue = PhysicalAddressAsPointer(asqFrame);
	driver->Queues[0].SubmissionTail = 0;
	driver->Registers->ACQ = acqFrame;
	driver->Queues[0].CompletionQueue = PhysicalAddressAsPointer(acqFrame);
	driver->Queues[0].CompletionHead = 0;
	driver->Queues[0].PhaseTag = 1;

	driver->Registers->AQA = (NVME_QUEUE_SIZE - 1) | ((NVME_QUEUE_SIZE - 1) << 16);

	driver->DoorbellStride = 4 << ((driver->Registers->CAP >> 32) & 0xf);
	driver->MinimumPageSize = 1 << (12 + ((driver->Registers->CAP >> 48) & 0xf));

	driver->Registers->CC |= 0x1;
	while ((driver->Registers->CSTS & 0x1) == 0)
		;
	SK_LOG_DEBUG("Successfully enabled the NVMe controller");

	// Identifying the controller, as per the recommended initialization sequence from the NVMe base spec
	PhysicalAddress identifyBuffer;
	AllocateFrame(&g_frameAllocator, &identifyBuffer);
	NVMeSubmissionEntry e = {};
	e.CDW0 = 0x6;
	e.PRP1 = identifyBuffer;
	e.CDW10 = 1;
	e.NSID = 0;

	NVMeSendAdminCommand(&g_nvmeDriver, &e);

	NVMeCompletionEntry c = {};
	result = NVMePollNextAdminCompletion(&g_nvmeDriver, &c);
	if (!result && !NVMeCommandCompletionStatus(&c)) {
		u64* a = (u64*)(identifyBuffer + g_bootInfo.PhysicalMemoryOffset);
		driver->MaximumTransferSize = (1 << ((*++a >> 13) & 0xf)) * driver->MinimumPageSize;
	}

	return ResultOk;
}

u16 NVMeCommandCompletionStatus(const NVMeCompletionEntry* entry) { return entry->STATUS >> 1; }

static void NVMeNotifySubmissionDoorbell(const NVMeDriver* driver, usz queueID)
{
	u8* doorbell = (u8*)driver->Registers;
	doorbell += 0x1000 + ((2 * queueID) * driver->DoorbellStride);
	*doorbell = driver->Queues[0].SubmissionTail;
}

static void NVMeNotifyCompletionDoorbell(const NVMeDriver* driver, u64 queueID)
{
	u8* doorbell = (u8*)driver->Registers;
	doorbell += 0x1000 + (((2 * queueID) + 1) * driver->DoorbellStride);
	*doorbell = driver->Queues[0].CompletionHead;
}

Result NVMeSendAdminCommand(NVMeDriver* driver, NVMeSubmissionEntry* command)
{
	MemoryCopy(command, &driver->Queues[0].SubmissionQueue[driver->Queues[0].SubmissionTail], sizeof(NVMeSubmissionEntry));
	driver->Queues[0].SubmissionTail = (driver->Queues[0].SubmissionTail + 1) % NVME_QUEUE_SIZE;
	NVMeNotifySubmissionDoorbell(driver, 0);

	return ResultOk;
}

Result NVMePollNextAdminCompletion(NVMeDriver* driver, NVMeCompletionEntry* entry)
{
	NVMeCompletionEntry* completion = &driver->Queues[0].CompletionQueue[driver->Queues[0].CompletionHead];

	// TODO: A timeout mechanism
	while (true) {
		if ((completion->STATUS & 0x1) != driver->Queues[0].PhaseTag) {
			__asm__ volatile("pause");
			continue;
		}

		// Copy the completion entry to the caller
		*entry = *completion;

		driver->Queues[0].CompletionHead = (driver->Queues[0].CompletionHead + 1) % NVME_QUEUE_SIZE;
		if (driver->Queues[0].CompletionHead == 0) {
			driver->Queues[0].PhaseTag ^= 1;
		}

		NVMeNotifyCompletionDoorbell(driver, 0);

		return ResultOk;
	}

	return ResultTimeout;
}
