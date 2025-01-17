#include "PCI.hpp"

#include "ACPI.hpp"
#include "Logger.hpp"
#include "Memory.hpp"
#include "Memory/Frame.hpp"
#include "Memory/Page.hpp"

namespace SaturnKernel {

PCIDevice g_pciStorageDevices[1];

namespace {
	auto MapEntireBus(PhysicalAddress baseAddress, U8 bus) -> Result<void>
	{
		Frame<Size4KiB> frame(baseAddress + (static_cast<U64>(bus) << 20));
		Page<Size4KiB> page(frame.Address.Value);

		// An entire bus is 256 4K pages, so 1 MB
		for (USIZE i = 0; i < 256; i++) {
			auto result = page.MapTo(frame, PageTableEntryFlags::Present | PageTableEntryFlags::Writeable);
			if (result.IsError()) {
				return result;
			}

			frame++;
			page++;
		}

		return Result<void>::MakeOk();
	}

	auto UnmapEntireBus(VirtualAddress baseAddress, U8 bus) -> Result<void>
	{
		Page<Size4KiB> page(baseAddress + (static_cast<U64>(bus) << 20));

		// An entire bus is 256 4K pages, so 1 MB
		for (USIZE i = 0; i < 256; i++) {
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
	auto EnumerateDevices(const MCFG::Entry* segmentGroup, U8 bus) -> USIZE
	{
		USIZE deviceCount = 0;

		for (U8 device = 0; device < 32; device++) {
			for (U8 function = 0; function < 8; function++) {
				VirtualAddress configSpaceAddress(segmentGroup->BaseAddress + (static_cast<U64>(bus) << 20)
					+ (static_cast<U64>(device) << 15) + (static_cast<U64>(function) << 12));
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
						.FunctionNumber = function };
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
	for (USIZE j = 0; j < mcfg->Entries(); j++) {
		MCFG::Entry* segmentGroup = mcfg->GetPCISegmentGroup(j);

		for (USIZE bus = segmentGroup->StartBusNumber; bus <= segmentGroup->EndBusNumber; bus++) {
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
