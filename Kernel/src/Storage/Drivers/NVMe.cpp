#include "Storage/Drivers/NVMe.hpp"

#include "Logger.hpp"
#include "Memory.hpp"
#include "Memory/BitmapFrameAllocator.hpp"
#include "PCI.hpp"

namespace SaturnKernel {

NVMeDriver g_nvmeDriver;

auto NVMeDriver::Init() -> Result<void>
{
	// For now, just using the first device and ignoring the rest ones
	g_pciStorageDevices[0].ConfigurationSpace->Command |= CommandRegister::EnableBusMaster | CommandRegister::EnableMemory;

	auto result = g_pciStorageDevices[0].SetMSIXVector(0, 34);
	if (result.IsError()) {
		return Result<void>::MakeErr(result.Error);
	}
	g_pciStorageDevices[0].EnableMSIX();

	auto barAddress = g_pciStorageDevices[0].BarAddress(0);
	if (barAddress.IsError()) {
		return Result<void>::MakeErr(barAddress.Error);
	}
	Registers = barAddress.Value.AsRawPointer<NVMeRegisters>();

	Registers->CC = 0;
	while ((Registers->CSTS & 0x1) != 0)
		;

	if ((Registers->CAP & (1UL << 37)) != 0 && (Registers->CAP & (1UL << 43)) == 0) {
		// NCSS
		Registers->CC &= ~(0b111 << 4);
	} else if ((Registers->CAP & (1UL << 43)) != 0) {
		// IOCSS
		Registers->CC |= 0b110 << 4;
	} else if ((Registers->CAP & (1UL << 44)) != 0) {
		// NOIOCSS
		Registers->CC |= 0b111 << 4;
	}

	// Set the Round Robin arbitration mechanism
	Registers->CC &= ~(0b1111 << 11);
	// Set the memory page size to 4 KiB
	Registers->CC &= ~(0b1111 << 7);

	auto asqFrame = g_frameAllocator.AllocateFrame();
	if (asqFrame.IsError()) {
		return Result<void>::MakeErr(asqFrame.Error);
	}
	auto acqFrame = g_frameAllocator.AllocateFrame();
	if (acqFrame.IsError()) {
		return Result<void>::MakeErr(acqFrame.Error);
	}

	// Just to be sure, zero out all of them
	// After this, the phase tag should be 1 on the first command completion
	MemoryFill(asqFrame.Value.UsableAddress(), 0, Frame<Size4KiB>::SIZE_BYTES);
	MemoryFill(acqFrame.Value.UsableAddress(), 0, Frame<Size4KiB>::SIZE_BYTES);

	Registers->ASQ = asqFrame.Value.Address.Value;
	AdminSubmissionQueue = asqFrame.Value.Address.AsPointer<NVMeSubmissionEntry>();
	Registers->ACQ = acqFrame.Value.Address.Value;
	AdminCompletionQueue = acqFrame.Value.Address.AsPointer<NVMeCompletionEntry>();

	Registers->AQA = (QUEUE_SIZE - 1) | ((QUEUE_SIZE - 1) << 16);

	DoorbellStride = 4 << ((Registers->CAP >> 32) & 0xf);

	Registers->CC |= 0x1;
	while ((Registers->CSTS & 0x1) == 0)
		;
	SK_LOG_DEBUG("Successfully enabled the NVMe controller");

	return Result<void>::MakeOk();
}

auto NVMeDriver::SendAdminCommand(NVMeSubmissionEntry command) -> Result<void>
{
	MemoryCopy(&command, &AdminSubmissionQueue[AdminSubmissionTail], sizeof(NVMeSubmissionEntry));
	AdminSubmissionTail = (AdminSubmissionTail + 1) % QUEUE_SIZE;

	u8* doorbell = reinterpret_cast<u8*>(Registers);
	doorbell += 0x1000 + ((2 * 0) * (DoorbellStride));
	*doorbell = AdminSubmissionTail;

	return Result<void>::MakeOk();
}

}
