#include "PCI.hpp"

#include "ACPI.hpp"
#include "APIC.hpp"
#include "Logger.hpp"
#include "MSR.hpp"
#include "Memory.hpp"
#include "Memory/Frame.hpp"
#include "Memory/Page.hpp"
#include "Panic.hpp"

namespace SaturnKernel {

PCIDevice g_pciStorageDevices[1];

auto PCIDevice::Init() -> Result<void>
{
	auto result = MapBars();
	if (result.IsError()) {
		return result;
	}

	if ((ConfigurationSpace->Status & StatusRegister::CapabilitiesList) != StatusRegister::CapabilitiesList) {
		return Result<void>::MakeErr(ErrorCode::PCICapabilitiesNotSupported);
	}

	u8* pciHeader = reinterpret_cast<u8*>(ConfigurationSpace);
	u8 capPtr = ConfigurationSpace->CapabilitiesPtr;

	while (capPtr != 0) {
		auto* capability = reinterpret_cast<DeviceCapability*>(pciHeader + capPtr);

		// TODO: Support MSI and not only MSI-X
		if (capability->ID == MSIXCapability::ID) {
			MSIX = reinterpret_cast<MSIXCapability*>(capability);
			break;
		}

		capPtr = capability->Next;
	}

	if (capPtr == 0) {
		return Result<void>::MakeErr(ErrorCode::SerialOutputUnavailabe);
	}

	u32 tableOffset = MSIX->TableOffset & ~0x7;
	u8 barIndex = MSIX->TableOffset & 0x7;

	auto bar = BarAddress(barIndex);
	if (bar.IsError()) {
		return Result<void>::MakeErr(bar.Error);
	}

	bar.Value += tableOffset;

	MSIXTable = bar.Value.AsRawPointer<MSIXTableEntry>();

	return Result<void>::MakeOk();
}

auto PCIDevice::MapBars() const -> Result<void>
{
	for (u8 i = 0; i < 6; i += 2) {
		// I do not support, I/O BARs
		if ((ConfigurationSpace->Bar[i] & 1) != 0 || ConfigurationSpace->Bar[i] == 0)
			continue;

		if ((ConfigurationSpace->Bar[i] & 0xf) != 4) {
			SK_LOG_WARN("Non 64-bit memory BAR found while mapping, skipping");
			// TODO: Support them :D
			continue;
		}

		const u32 barLow = ConfigurationSpace->Bar[i];
		const u32 barHigh = ConfigurationSpace->Bar[i + 1];

		const PhysicalAddress address(static_cast<u64>(barLow & 0xFFFFFFF0) | (static_cast<u64>(barHigh) << 32));

		ConfigurationSpace->Bar[i] = 0xFFFFFFFF;
		ConfigurationSpace->Bar[i + 1] = 0xFFFFFFFF;

		const u32 barSizeLow = ConfigurationSpace->Bar[i];
		const u32 barSizeHigh = ConfigurationSpace->Bar[i + 1];

		const u64 sizeMask = (static_cast<u64>(barSizeLow & 0xFFFFFFF0) | (static_cast<u64>(barSizeHigh) << 32));
		const u64 barSize = ~sizeMask + 1;

		ConfigurationSpace->Bar[i] = barLow;
		ConfigurationSpace->Bar[i + 1] = barHigh;

		for (Page<Size4KiB> page(address.Value); page < Page<Size4KiB>(address.Value + barSize); page++) {
			// TODO: If prefetchable enable cache
			auto result = page.MapTo(Frame<Size4KiB>(page.Address.Value),
				PageTableEntryFlags::Present | PageTableEntryFlags::Writeable | PageTableEntryFlags::NoCache);
			if (result.IsError()) {
				// This is an extremally dangerous approach but since the kernel will panic later,
				// I'll jsut leave it like this for now
				return result;
			}
		}
	}

	return Result<void>::MakeOk();
}

auto PCIDevice::BarAddress(u8 index) const -> Result<PhysicalAddress>
{
	for (u8 i = 0; i < 6; i += 2) {
		// I do not support, I/O BARs
		if ((ConfigurationSpace->Bar[i] & 1) != 0 || ConfigurationSpace->Bar[i] == 1)
			continue;

		if ((ConfigurationSpace->Bar[i] & 0xf) != 4) {
			continue;
		}

		const u32 barLow = ConfigurationSpace->Bar[i];
		const u32 barHigh = ConfigurationSpace->Bar[i + 1];

		const PhysicalAddress address(static_cast<u64>(barLow & 0xFFFFFFF0) | (static_cast<u64>(barHigh) << 32));

		if (index == i)
			return Result<PhysicalAddress>::MakeOk(address);
	}

	return Result<PhysicalAddress>::MakeErr(ErrorCode::InvalidBARIndex);
}

auto PCIDevice::EnableMSIX() const -> void { MSIX->MessageControl |= 0x8000; }

auto PCIDevice::SetMSIXVector(usize msiVector, u8 systemVector) const -> Result<void>
{
	if (msiVector > (MSIX->MessageControl & 0x7ff)) {
		return Result<void>::MakeErr(ErrorCode::InvalidMSIXVector);
	}

	u64 lapicID = ReadMSR(LAPIC::ID_REGISTER_MSR);
	u32 addressLow = 0xfee00000 | (1 << 3) | ((lapicID & 0xff) << 12);
	u32 addressHigh = 0x0 | ((lapicID >> 8) & 0xffffff);
	u32 data = systemVector;

	MSIXTable[msiVector].AddressLow = addressLow;
	MSIXTable[msiVector].AddressHigh = addressHigh;
	MSIXTable[msiVector].Data = data;
	MSIXTable[msiVector].VectorControl = 0;

	return Result<void>::MakeOk();
}

namespace {
	auto MapEntireBus(PhysicalAddress baseAddress, u8 bus) -> Result<void>
	{
		Frame<Size4KiB> frame(baseAddress + (static_cast<u64>(bus) << 20));
		Page<Size4KiB> page(frame.Address.Value);

		// An entire bus is 256 4K pages, so 1 MB
		for (usize i = 0; i < 256; i++) {
			auto result = page.MapTo(frame, PageTableEntryFlags::Present | PageTableEntryFlags::Writeable);
			if (result.IsError()) {
				return result;
			}

			frame++;
			page++;
		}

		return Result<void>::MakeOk();
	}

	auto UnmapEntireBus(VirtualAddress baseAddress, u8 bus) -> Result<void>
	{
		Page<Size4KiB> page(baseAddress + (static_cast<u64>(bus) << 20));

		// An entire bus is 256 4K pages, so 1 MB
		for (usize i = 0; i < 256; i++) {
			auto result = page.Unmap();
			if (result.IsError()) {
				return result;
			}

			FlushPage(page.Address);

			page++;
		}

		return Result<void>::MakeOk();
	}

	/// Saves the devices I care about for later use and returns how many of them were detected.
	auto EnumerateDevices(const MCFG::Entry* segmentGroup, u8 bus) -> usize
	{
		usize deviceCount = 0;

		for (u8 device = 0; device < 32; device++) {
			for (u8 function = 0; function < 8; function++) {
				VirtualAddress configSpaceAddress(segmentGroup->BaseAddress + (static_cast<u64>(bus) << 20)
					+ (static_cast<u64>(device) << 15) + (static_cast<u64>(function) << 12));
				auto* configSpace = configSpaceAddress.AsPointer<PCIDeviceHeader0>();

				if (configSpace->VendorID == 0xffff)
					continue;

				SK_LOG_DEBUG("Detected PCI Device: bus = {}, device = {}, function = {}, vendorID = {}, deviceID = {}, class = {}, "
							 "subclass = {}",
					bus, device, function, configSpace->VendorID, configSpace->DeviceID, configSpace->ClassCode, configSpace->Subclass);

				if (configSpace->ClassCode == 0x1 && configSpace->Subclass == 0x8 && configSpace->ProgIF == 0x2) {
					g_pciStorageDevices[0] = { .ConfigurationSpace = configSpace,
						.PCISegmentGroupNumber = segmentGroup->SegmentGroupNumber,
						.BusNumber = bus,
						.DeviceNumber = device,
						.FunctionNumber = function,
						.MSIX = nullptr,
						.MSIXTable = nullptr };
					auto result = g_pciStorageDevices[0].Init();
					SK_PANIC_ASSERT(result.IsOk(), "An error occured while trying to initialize a PCI Device");
				}

				deviceCount++;
			}
		}

		return deviceCount;
	}
}

auto ScanPCIDevices() -> Result<void>
{
	auto mcfgAddress = g_xsdt->GetACPITableAddress("MCFG");
	if (mcfgAddress.IsError()) {
		return Result<void>::MakeErr(mcfgAddress.Error);
	}

	auto* mcfg = mcfgAddress.Value.AsPointer<MCFG>();

	// In each PCI segment group, I map the entire bus's extended configuration spcae
	// and if there are no devices there, I unmap it to not waste memory.
	for (usize j = 0; j < mcfg->Entries(); j++) {
		MCFG::Entry* segmentGroup = mcfg->GetPCISegmentGroup(j);

		for (usize bus = segmentGroup->StartBusNumber; bus <= segmentGroup->EndBusNumber; bus++) {
			auto result = MapEntireBus(PhysicalAddress(segmentGroup->BaseAddress), bus);
			if (result.IsError())
				return Result<void>::MakeErr(result.Error);

			// If we didn't find any devices in the bus's extended configuration space, unmap it.
			if (EnumerateDevices(segmentGroup, bus) <= 0) {
				result = UnmapEntireBus(VirtualAddress(segmentGroup->BaseAddress), bus);

				if (result.IsError())
					return Result<void>::MakeErr(result.Error);
			}
		}
	}

	return Result<void>::MakeOk();
}

}
