#include "Core.h"
#include "Memory/PhysicalAddress.h"
#include "Result.h"

typedef enum CommandRegister : u16 {
	CommandRegisterEnableIO = 1,
	CommandRegisterEnableMemory = 1 << 1,
	CommandRegisterEnableBusMaster = 1 << 2,
	CommandRegisterSpecialCycles = 1 << 3,
	CommandRegisterMemoryWriteInvalidate = 1 << 4,
	CommandRegisterVGAPaletteSnoop = 1 << 5,
	CommandRegisterParityErrorResponse = 1 << 6,
	CommandRegisterEnableSERR = 1 << 8,
	CommandRegisterEnableFastB2B = 1 << 9,
	CommandRegisterDisableInterrupts = 1 << 10,
} CommandRegister;

typedef enum StatusRegister : u16 {
	StatusRegisterInterruptStatus = 1 << 3,
	StatusRegisterCapabilitiesList = 1 << 4,
	StatusRegisterCapable66MHz = 1 << 5,
	StatusRegisterFastB2BCaoable = 1 << 7,
	StatusRegisterMasterDataParityError = 1 << 8,
	StatusRegisterSignaledTargetAbort = 1 << 11,
	StatusRegisterReceivedTargetAbort = 1 << 12,
	StatusRegisterReceivedMasterAbort = 1 << 13,
	StatusRegisterSignaledSystemError = 1 << 14,
	StatusRegisterDetectedParityError = 1 << 15,
} StatusRegister;

typedef struct PCIDeviceHeader0 {
	u16 VendorID;
	u16 DeviceID;
	CommandRegister Command;
	StatusRegister Status;
	u8 RevisionID;
	u8 ProgIF;
	u8 Subclass;
	u8 ClassCode;
	u8 CacheLineSize;
	u8 LatencyTimer;
	u8 HeaderType;
	u8 Bist;
	u32 Bar[6];
	u32 CardbusCisPointer;
	u16 SubsystemVendorID;
	u16 SubsystemID;
	u32 ExpansionRomBase;
	u8 CapabilitiesPtr;
	u8 Reserved[7];
	u8 InterruptLine;
	u8 InterruptPin;
	u8 MinGrant;
	u8 MaxLatency;
} PCIDeviceHeader0;

typedef struct DeviceCapability {
	u8 ID;
	u8 Next;
} DeviceCapability;

typedef struct MSIXCapability {
	DeviceCapability BaseCapability;

	u16 MessageControl;
	u32 TableOffset;
	u32 PendingBitOffset;
} MSIXCapability;

constexpr u8 CAPABILITY_ID_MSIX = 0x11;

typedef struct MSIXTableEntry {
	u32 AddressLow;
	u32 AddressHigh;
	u32 Data;
	u32 VectorControl;
} MSIXTableEntry;

typedef struct PCIDevice {
	PCIDeviceHeader0* ConfigurationSpace;

	u16 PCISegmentGroupNumber;
	u8 BusNumber;
	u8 DeviceNumber;
	u8 FunctionNumber;

	MSIXCapability* MSIX;
	MSIXTableEntry* MSIXTable;
} PCIDevice;

Result PCIDeviceInit(PCIDevice* device);

Result PCIDeviceMapBars(const PCIDevice* device);
Result PCIDeviceBarAddress(const PCIDevice* device, u8 index, PhysicalAddress* address);

void PCIDeviceEnableMSIX(const PCIDevice* device);
Result PCIDeviceSetMSIXVector(const PCIDevice* device, usz msiVector, u8 systemVector);

Result ScanPCIDevices();

// For now, this is it. In the future when I will actually start using more devices, I will make a proper device list and so on...
extern PCIDevice g_pciStorageDevices[1];
