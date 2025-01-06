#include "Core.hpp"
#include "Result.hpp"

namespace SaturnKernel {

struct PCIDeviceHeader0 {
	U16 VendorID;
	U16 DeviceID;
	U16 Command;
	U16 Status;
	U8 RevisionID;
	U8 ProgIF;
	U8 Subclass;
	U8 ClassCode;
	U8 CacheLineSize;
	U8 LatencyTimer;
	U8 HeaderType;
	U8 Bist;
	U32 Bar[6];
	U32 CardbusCisPointer;
	U16 SubsystemVendorID;
	U16 SubsystemID;
	U32 ExpansionRomBase;
	U8 CapabilitiesPtr;
	U8 Reserved[7];
	U8 InterruptLine;
	U8 InterruptPin;
	U8 MinGrant;
	U8 MaxLatency;
};

struct PCIDevice {
	PCIDeviceHeader0* ConfigurationSpace;

	U16 PCISegmentGroupNumber;
	U8 BusNumber;
	U8 DeviceNumber;
	U8 FunctionNumber;
};

auto ScanPCIDevices() -> Result<void>;

// For now, this is it. In the future when I will actually start using more devices, I will make a proper device list and so on...
extern PCIDevice g_pciStorageDevices[1];

}
