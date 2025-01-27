#include "Storage/Drivers/NVMe.h"

#include "Logger.h"
#include "Memory.h"
#include "Memory/BitmapFrameAllocator.h"
#include "PCI.h"

NVMeDriver g_nvmeDriver;

Result NVMeInit(NVMeDriver* driver)
{
	// For now, just using the first device and ignoring the rest ones
	g_pciStorageDevices[0].ConfigurationSpace->Command |= CommandRegisterEnableBusMaster | CommandRegisterEnableMemory;

	Result result = PCIDeviceSetMSIXVector(&g_pciStorageDevices[0], 0, 34);
	if (result) {
		return result;
	}
	PCIDeviceEnableMSIX(&g_pciStorageDevices[0]);

	PhysicalAddress barAddress;
	result = PCIDeviceBarAddress(&g_pciStorageDevices[0], 0, &barAddress);
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
	driver->Registers->CC &= ~(0b1111 << 11);
	// Set the memory page size to 4 KiB
	driver->Registers->CC &= ~(0b1111 << 7);

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

	driver->Registers->ASQ = asqFrame;
	driver->AdminSubmissionQueue = PhysicalAddressAsPointer(asqFrame);
	driver->Registers->ACQ = acqFrame;
	driver->AdminCompletionQueue = PhysicalAddressAsPointer(acqFrame);

	driver->Registers->AQA = (NVME_QUEUE_SIZE - 1) | ((NVME_QUEUE_SIZE - 1) << 16);

	driver->DoorbellStride = 4 << ((driver->Registers->CAP >> 32) & 0xf);

	driver->Registers->CC |= 0x1;
	while ((driver->Registers->CSTS & 0x1) == 0)
		;
	SK_LOG_DEBUG("Successfully enabled the NVMe controller");

	return ResultOk;
}

Result NVMeSendAdminCommand(NVMeDriver* driver, NVMeSubmissionEntry command)
{
	MemoryCopy(&command, &driver->AdminSubmissionQueue[driver->AdminSubmissionTail], sizeof(NVMeSubmissionEntry));
	driver->AdminSubmissionTail = (driver->AdminSubmissionTail + 1) % NVME_QUEUE_SIZE;

	u8* doorbell = (u8*)(driver->Registers);
	doorbell += 0x1000 + ((2 * 0) * (driver->DoorbellStride));
	*doorbell = driver->AdminSubmissionTail;

	return ResultOk;
}
