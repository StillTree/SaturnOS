#include "Storage/Drivers/AHCI.h"

#include "Memory.h"
#include "Memory/BitmapFrameAllocator.h"
#include "PCI.h"

AHCIDriver g_ahciDriver;

static u8 AHCIDeviceFindCommandListSlot(AHCIDevice* device)
{
	while (true) {
		for (u8 i = 0; i < 32; i++) {
			if (!((device->Registers->CI >> i) & 1)) {
				return i;
			}
		}
	}
}

static AHCICommandHeader* AHCIDeviceGetCommandHeaderAddress(AHCIDevice* device, u8 commandSlot)
{
	return device->CommandList + commandSlot;
}

static void AHCIDeviceSubmitCommand(AHCIDevice* device, u8 commandSlot) { device->Registers->CI |= 1 << commandSlot; }

static void AHCIDevicePollCommandCompletion(AHCIDevice* device, u8 commandSlot)
{
	while ((device->Registers->CI >> commandSlot) & 1)
		;
}

static Result AHCIDeviceAllocateCommandTables(AHCIDevice* device)
{
	for (u8 i = 0; i < 32; i++) {
		AHCICommandHeader* command = device->CommandList + i;

		// I allocate a single frame, even though this isn't technically enough for an entire command table
		Frame4KiB frame;
		Result result = AllocateFrame(&g_frameAllocator, &frame);
		if (result) {
			return result;
		}

		command->CTBA = frame & 0xffffffff;
		command->CTBAU = frame >> 32;
	}

	return ResultOk;
}

static AHCICommandTable* AHCICommandHeaderGetCommandTable(AHCICommandHeader* commandHeader)
{
	return PhysicalAddressAsPointer(((u64)commandHeader->CTBAU << 32) | commandHeader->CTBA);
}

static void AHCICommandTableReset(AHCICommandTable* commandTable)
{
	// I only clear a single frame because I only allocated a single frame there
	MemoryFill(commandTable, 0, FRAME_4KIB_SIZE_BYTES);
}

Result AHCIReset(AHCIDriver* ahci)
{
	// Reset the HBA
	ahci->Registers->GHC |= 1;
	while (ahci->Registers->GHC & 1)
		;

	// Disable interrupts
	ahci->Registers->GHC &= ~(1 << 1);
	// Enable AHCI mode after reset
	ahci->Registers->GHC &= ~(1 << 31);

	// Check for 64-bit addressing support
	if (!((ahci->Registers->GHC >> 31) & 1)) {
		return ResultAHCI64BitAddressingUnsupported;
	}

	return ResultOk;
}

/// Helper function for filling out the devices sector information.
static Result AHCIDeviceIdentify(AHCIDevice* device)
{
	u8 commandSlot = AHCIDeviceFindCommandListSlot(device);
	AHCICommandHeader* commandHeader = AHCIDeviceGetCommandHeaderAddress(device, commandSlot);

	commandHeader->CFL = FIS_DWORD_LENGTH_REGISTER_H2D;
	commandHeader->PRDTL = 1;

	AHCICommandTable* commandTable = AHCICommandHeaderGetCommandTable(commandHeader);

	PhysicalAddress identifyFrame;
	AllocateFrame(&g_frameAllocator, &identifyFrame);
	MemoryFill(PhysicalAddressAsPointer(identifyFrame), 0, FRAME_4KIB_SIZE_BYTES);

	commandTable->PRDT[0].DBA = identifyFrame & 0xffffffff;
	commandTable->PRDT[0].DBAU = identifyFrame >> 32;
	commandTable->PRDT[0].DBC = 511;
	commandTable->PRDT[0].Reserved = 0;

	FISRegisterH2D* commandFIS = (FISRegisterH2D*)&commandTable->CFIS[0];
	commandFIS->FISType = FISTypeRegisterH2D;
	commandFIS->PortMultiplier = 1 << 7;
	commandFIS->Command = ATACommandIdentifyDevice;

	AHCIDeviceSubmitCommand(device, commandSlot);

	AHCIDevicePollCommandCompletion(device, commandSlot);

	u16* identifyDataRaw = PhysicalAddressAsPointer(identifyFrame);

	// Indicates whether this drive's sectors are longer than 512 bytes
	// If they are, I just error out 'cause I'm lazy :D
	if ((identifyDataRaw[106] >> 12) & 1) {
		return ResultAHCIDeviceUnsupportedSectorSize;
	}

	device->SectorCount = identifyDataRaw[100] | (identifyDataRaw[101] << 16);
	device->SectorSize = 512;

	Result result = DeallocateFrame(&g_frameAllocator, identifyFrame);
	if (result) {
		return result;
	}

	return ResultOk;
}

Result AHCIDeviceInit(AHCIDevice* device)
{
	// Stop processing Commands
	device->Registers->CMD &= ~1;
	// Stop FIS receiving
	device->Registers->CMD &= ~(1 << 4);
	// Wait for any remaining Command processing and FIS receving to stop
	while ((device->Registers->CMD >> 15) & 1 || (device->Registers->CMD >> 14) & 1)
		;

	// Clear all ATA errors
	device->Registers->SERR = ~0;

	PhysicalAddress receivedFisFrame;
	Result result = AllocateFrame(&g_frameAllocator, &receivedFisFrame);
	if (result) {
		return result;
	}
	device->Registers->FB = receivedFisFrame & 0xffffffff;
	device->Registers->FBU = receivedFisFrame >> 32;

	PhysicalAddress commandListFrame;
	result = AllocateFrame(&g_frameAllocator, &commandListFrame);
	if (result) {
		return result;
	}
	device->Registers->CLB = commandListFrame & 0xffffffff;
	device->Registers->CLBU = commandListFrame >> 32;

	device->CommandList = PhysicalAddressAsPointer(commandListFrame);

	result = AHCIDeviceAllocateCommandTables(device);
	if (result) {
		return result;
	}

	// The AHCI specification instructs to wait here for any remaining Command processing and FIS receving to stop,
	// but since I do that earlier in the code I don't have to do that here

	// Start FIS receiving
	device->Registers->CMD |= 1 << 4;
	// Start processing Commands
	device->Registers->CMD |= 1;

	result = AHCIDeviceIdentify(device);
	if (result) {
		return result;
	}

	return ResultOk;
}

Result AHCIDeviceReadSectors(AHCIDevice* device, usz startSectorIndex, u16 sectorCount, PhysicalAddress buffer)
{
	u8 commandSlot = AHCIDeviceFindCommandListSlot(device);
	AHCICommandHeader* commandHeader = AHCIDeviceGetCommandHeaderAddress(device, commandSlot);

	AHCICommandTable* commandTable = AHCICommandHeaderGetCommandTable(commandHeader);

	AHCICommandTableReset(commandTable);

	commandHeader->CFL = FIS_DWORD_LENGTH_REGISTER_H2D;
	commandHeader->PRDTL = 1;

	commandTable->PRDT[0].DBA = buffer & 0xffffffff;
	commandTable->PRDT[0].DBAU = buffer >> 32;
	commandTable->PRDT[0].DBC = (sectorCount * device->SectorSize) - 1;

	FISRegisterH2D* commandFIS = (FISRegisterH2D*)&commandTable->CFIS[0];
	commandFIS->FISType = FISTypeRegisterH2D;
	commandFIS->PortMultiplier = 1 << 7;
	commandFIS->Command = ATACommandReadDMAExtended;
	commandFIS->LBA0 = startSectorIndex & 0xff;
	commandFIS->LBA1 = (startSectorIndex >> 8) & 0xff;
	commandFIS->LBA2 = (startSectorIndex >> 16) & 0xff;
	commandFIS->LBA3 = (startSectorIndex >> 24) & 0xff;
	commandFIS->LBA4 = (startSectorIndex >> 32) & 0xff;
	commandFIS->LBA5 = (startSectorIndex >> 40) & 0xff;
	commandFIS->Count = sectorCount;
	commandFIS->Device = 1 << 6;

	AHCIDeviceSubmitCommand(device, commandSlot);

	AHCIDevicePollCommandCompletion(device, commandSlot);

	return ResultOk;
}

Result AHCIDeviceWriteSectors(AHCIDevice* device, usz startSectorIndex, usz sectorCount, PhysicalAddress buffer)
{
	u8 commandSlot = AHCIDeviceFindCommandListSlot(device);
	AHCICommandHeader* commandHeader = AHCIDeviceGetCommandHeaderAddress(device, commandSlot);

	AHCICommandTable* commandTable = AHCICommandHeaderGetCommandTable(commandHeader);

	AHCICommandTableReset(commandTable);

	commandHeader->CFL = FIS_DWORD_LENGTH_REGISTER_H2D;
	commandHeader->CFL |= AHCICommandWrite;
	commandHeader->PRDTL = 1;

	commandTable->PRDT[0].DBA = buffer & 0xffffffff;
	commandTable->PRDT[0].DBAU = buffer >> 32;
	commandTable->PRDT[0].DBC = (sectorCount * device->SectorSize) - 1;

	FISRegisterH2D* commandFIS = (FISRegisterH2D*)&commandTable->CFIS[0];
	commandFIS->FISType = FISTypeRegisterH2D;
	commandFIS->PortMultiplier = 1 << 7;
	commandFIS->Command = ATACommandWriteDMAExtended;
	commandFIS->LBA0 = startSectorIndex & 0xff;
	commandFIS->LBA1 = (startSectorIndex >> 8) & 0xff;
	commandFIS->LBA2 = (startSectorIndex >> 16) & 0xff;
	commandFIS->LBA3 = (startSectorIndex >> 24) & 0xff;
	commandFIS->LBA4 = (startSectorIndex >> 32) & 0xff;
	commandFIS->LBA5 = (startSectorIndex >> 40) & 0xff;
	commandFIS->Count = sectorCount;
	commandFIS->Device = 1 << 6;

	AHCIDeviceSubmitCommand(device, commandSlot);

	AHCIDevicePollCommandCompletion(device, commandSlot);

	return ResultOk;
}

Result InitAHCI()
{
	g_pciStorageDevices[0].ConfigurationSpace->Command |= CommandRegisterEnableBusMaster | CommandRegisterEnableMemory;

	PhysicalAddress hbaRegistersAddress;
	Result result = PCIDeviceBarAddress(&g_pciStorageDevices[0], 5, &hbaRegistersAddress);
	if (result) {
		return result;
	}
	g_ahciDriver.Registers = (HBARegisters*)hbaRegistersAddress;

	AHCIReset(&g_ahciDriver);
	for (u8 i = 0; i < 32; i++) {
		if (!((g_ahciDriver.Registers->PI >> i) & 1) || !(g_ahciDriver.Registers->Ports[i].SSTS & HBA_PORT_DEVICE_PRESENT))
			continue;

		g_ahciDriver.Devices[0].Registers = g_ahciDriver.Registers->Ports + i;
		g_ahciDriver.Devices[0].Port = i;
		result = AHCIDeviceInit(&g_ahciDriver.Devices[0]);
		if (result) {
			return result;
		}
	}

	return ResultOk;
}
