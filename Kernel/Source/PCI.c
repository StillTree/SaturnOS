#include "PCI.h"

#include "ACPI.h"
#include "APIC.h"
#include "Logger.h"
#include "Memory/Frame.h"
#include "Memory/Page.h"
#include "Memory/VirtualMemoryAllocator.h"
#include "Panic.h"

PCIDevice g_pciStorageDevices[1];

Result PCIDeviceInit(PCIDevice* device)
{
	Result result = PCIDeviceMapBars(device);
	if (result) {
		return result;
	}

	// if (!(device->ConfigurationSpace->Status & StatusRegisterCapabilitiesList)) {
	// 	return ResultPCICapabilitiesNotSupported;
	// }

	// u8* pciHeader = (u8*)device->ConfigurationSpace;
	// u8 capPtr = device->ConfigurationSpace->CapabilitiesPtr;

	// while (capPtr != 0) {
	// 	DeviceCapability* capability = (DeviceCapability*)(pciHeader + capPtr);

	// 	// TODO: Support MSI and not only MSI-X
	// 	if (capability->ID == CAPABILITY_ID_MSIX) {
	// 		device->MSIX = (MSIXCapability*)(capability);
	// 		break;
	// 	}

	// 	capPtr = capability->Next;
	// }

	// // If we didn't break from the loop earlier, this means that MSI-X is not supported
	// if (capPtr == 0) {
	// 	return ResultSerialOutputUnavailabe;
	// }

	// u32 tableOffset = device->MSIX->TableOffset & ~0x7;
	// u8 barIndex = device->MSIX->TableOffset & 0x7;

	// PhysicalAddress barAddress;
	// result = PCIDeviceBarAddress(device, barIndex, &barAddress);
	// if (result) {
	// 	return result;
	// }

	// barAddress += tableOffset;

	// device->MSIXTable = (MSIXTableEntry*)barAddress;

	return ResultOk;
}

Result PCIDeviceMapBars(PCIDevice* device)
{
	for (u8 i = 0; i < 6; i++) {
		// I do not support, I/O BARs
		if ((device->ConfigurationSpace->Bar[i] & 1) != 0 || device->ConfigurationSpace->Bar[i] == 0)
			continue;

		if ((device->ConfigurationSpace->Bar[i] & 0xf) != 4) {
			const u32 bar = device->ConfigurationSpace->Bar[i];

			PhysicalAddress address = bar & 0xfffffff0;

			device->ConfigurationSpace->Bar[i] = 0xffffffff;

			const u32 barSize = device->ConfigurationSpace->Bar[i];

			const u32 sizeMask = barSize & 0xfffffff0;
			const u32 size = ~sizeMask + 1;

			device->ConfigurationSpace->Bar[i] = bar;

			Page4KiB mappedBar;
			Result result = AllocateMMIORegion(&g_kernelMemoryAllocator, address, size, PageWriteable | PageNoCache, &mappedBar);
			if (result) {
				// This is an extremally dangerous approach but since the kernel will panic later,
				// I'll just leave it like this for now
				return result;
			}

			device->MostUsefulBAR = mappedBar;

			SK_LOG_DEBUG("Mapped 32-bit BAR with virtual address: 0x%x and size: 0x%x", mappedBar, size);

			continue;
		}

		const u32 barLow = device->ConfigurationSpace->Bar[i];
		const u32 barHigh = device->ConfigurationSpace->Bar[i + 1];

		PhysicalAddress address = (u64)(barLow & 0xfffffff0) | ((u64)(barHigh) << 32);

		device->ConfigurationSpace->Bar[i] = 0xffffffff;
		device->ConfigurationSpace->Bar[i + 1] = 0xffffffff;

		const u32 barSizeLow = device->ConfigurationSpace->Bar[i];
		const u32 barSizeHigh = device->ConfigurationSpace->Bar[i + 1];

		const u64 sizeMask = ((u64)(barSizeLow & 0xfffffff0) | ((u64)barSizeHigh << 32));
		const u64 size = ~sizeMask + 1;

		device->ConfigurationSpace->Bar[i] = barLow;
		device->ConfigurationSpace->Bar[i + 1] = barHigh;

		Page4KiB mappedBar;
		Result result = AllocateMMIORegion(&g_kernelMemoryAllocator, address, size, PageWriteable | PageNoCache, &mappedBar);
		if (result) {
			// This is an extremally dangerous approach but since the kernel will panic later,
			// I'll just leave it like this for now
			return result;
		}

		device->MostUsefulBAR = mappedBar;

		SK_LOG_DEBUG("Mapped 64-bit BAR with virtual address: 0x%x and size: 0x%x", address, size);
	}

	return ResultOk;
}

// Result PCIDeviceBarAddress(const PCIDevice* device, u8 index, PhysicalAddress* address)
// {
// 	// I do not support, I/O BARs
// 	if ((device->ConfigurationSpace->Bar[index] & 1) != 0 || device->ConfigurationSpace->Bar[index] == 1)
// 		return ResultInvalidBARIndex;
//
// 	// 32-bit BAR
// 	if ((device->ConfigurationSpace->Bar[index] & 0xf) != 4) {
// 		u32 bar = device->ConfigurationSpace->Bar[index];
//
// 		const PhysicalAddress barAddress = bar & 0xfffffff0;
//
// 		*address = barAddress;
// 		return ResultOk;
// 	}
//
// 	// 64-bit BAR
// 	const u32 barLow = device->ConfigurationSpace->Bar[index];
// 	const u32 barHigh = device->ConfigurationSpace->Bar[index + 1];
//
// 	const PhysicalAddress barAddress = (u64)(barLow & 0xfffffff0) | ((u64)barHigh << 32);
//
// 	*address = barAddress;
// 	return ResultOk;
// }

void PCIDeviceEnableMSIX(const PCIDevice* device) { device->MSIX->MessageControl |= 0x8000; }

Result PCIDeviceSetMSIXVector(const PCIDevice* device, usz msiVector, u8 systemVector)
{
	if (msiVector > (device->MSIX->MessageControl & 0x7ff)) {
		return ResultInvalidMSIXVector;
	}

	// TODO: Do not hardcode these values here
	u64 lapicID = LAPICReadRegister(LAPIC_ID_REGISTER);
	u32 addressLow = 0xfee00000 | (1 << 3) | ((lapicID & 0xff) << 12);
	u32 addressHigh = 0x0 | ((lapicID >> 8) & 0xffffff);
	u32 data = systemVector;

	device->MSIXTable[msiVector].AddressLow = addressLow;
	device->MSIXTable[msiVector].AddressHigh = addressHigh;
	device->MSIXTable[msiVector].Data = data;
	device->MSIXTable[msiVector].VectorControl = 0;

	return ResultOk;
}

static Result MapEntireBus(PhysicalAddress baseAddress, u8 bus, Page4KiB* mappedBus)
{
	Frame4KiB frame = baseAddress + ((u64)bus << 20);

	Result result = AllocateMMIORegion(&g_kernelMemoryAllocator, frame, 0x100000, PageWriteable, mappedBus);
	if (result) {
		return result;
	}

	return result;
}

static Result UnmapEntireBus(Page4KiB mappedBus)
{
	Result result = DeallocateMMIORegion(&g_kernelMemoryAllocator, mappedBus, 0x100000);
	if (result) {
		return result;
	}

	return ResultOk;
}

/// Saves the devices I care about for later use and returns how many of them were detected.
static usz EnumerateDevices(const MCFGEntry* segmentGroup, u8 bus, Page4KiB mappedBus)
{
	usz deviceCount = 0;

	for (u8 device = 0; device < 32; device++) {
		for (u8 function = 0; function < 8; function++) {
			VirtualAddress configSpaceAddress = mappedBus + ((u64)device << 15) + ((u64)function << 12);
			PCIDeviceHeader0* configSpace = (PCIDeviceHeader0*)configSpaceAddress;

			if (configSpace->VendorID == 0xffff)
				continue;

			SK_LOG_DEBUG("Detected PCI Device: device = %u, function = %u, VendorID = %x, DeviceID = %x, ClassCode = %x, "
						 "Subclass = %x, ProgIF = %x",
				(u64)device, (u64)function, (u64)configSpace->VendorID, (u64)configSpace->DeviceID, (u64)configSpace->ClassCode,
				(u64)configSpace->Subclass, (u64)configSpace->ProgIF);

			// The function is a temporary thing, later I plan on supporting multiple devices of course
			if (configSpace->ClassCode == 0x1 && configSpace->Subclass == 0x6 && configSpace->ProgIF == 0x1 && function == 0) {
				PCIDevice ahciController = { .ConfigurationSpace = configSpace,
					.PCISegmentGroupNumber = segmentGroup->SegmentGroupNumber,
					.BusNumber = bus,
					.DeviceNumber = device,
					.FunctionNumber = function,
					.MSIX = nullptr,
					.MSIXTable = nullptr };
				g_pciStorageDevices[0] = ahciController;
				SK_PANIC_ON_ERROR(
					PCIDeviceInit(&g_pciStorageDevices[0]), "An error occured while trying to initialize the PCI storage device");
			}

			deviceCount++;
		}
	}

	return deviceCount;
}

Result ScanPCIDevices()
{
	PhysicalAddress mcfgAddress;
	Result result = GetACPITableAddress("MCFG", &mcfgAddress);
	if (result) {
		return result;
	}

	MCFG* mcfg = PhysicalAddressAsPointer(mcfgAddress);

	// In each PCI segment group, I map the entire bus's extended configuration spcae
	// and if there are no devices there, I unmap it to not waste memory.
	for (usz i = 0; i < MCFGEntries(mcfg); i++) {
		MCFGEntry* segmentGroup = &mcfg->Entries[i];

		for (usz bus = segmentGroup->StartBusNumber; bus <= segmentGroup->EndBusNumber; bus++) {
			Page4KiB mappedBus;
			Result result = MapEntireBus(segmentGroup->BaseAddress, bus, &mappedBus);
			if (result)
				return result;

			// If we didn't find any devices in the bus's extended configuration space, unmap it.
			if (EnumerateDevices(segmentGroup, bus, mappedBus) <= 0) {
				result = UnmapEntireBus(mappedBus);

				if (result)
					return result;
			}
		}
	}

	return ResultOk;
}
