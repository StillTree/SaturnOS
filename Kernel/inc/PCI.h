#include "Core.h"
#include "Memory/PhysicalAddress.h"
#include "Result.h"

namespace SaturnKernel {

enum class CommandRegister : u16 {
	EnableIO = 1,
	EnableMemory = 1 << 1,
	EnableBusMaster = 1 << 2,
	SpecialCycles = 1 << 3,
	MemoryWriteInvalidate = 1 << 4,
	VGAPaletteSnoop = 1 << 5,
	ParityErrorResponse = 1 << 6,
	EnableSERR = 1 << 8,
	EnableFastB2B = 1 << 9,
	DisableInterrupts = 1 << 10,
};

inline auto operator|(CommandRegister a, CommandRegister b) -> CommandRegister
{
	return static_cast<CommandRegister>(static_cast<u16>(a) | static_cast<u16>(b));
}

inline auto operator&(CommandRegister a, CommandRegister b) -> CommandRegister
{
	return static_cast<CommandRegister>(static_cast<u16>(a) & static_cast<u16>(b));
}

inline auto operator|=(CommandRegister& a, CommandRegister b) -> CommandRegister& { return a = a | b; }

enum class StatusRegister : u16 {
	InterruptStatus = 1 << 3,
	CapabilitiesList = 1 << 4,
	Capable66MHz = 1 << 5,
	FastB2BCaoable = 1 << 7,
	MasterDataParityError = 1 << 8,
	SignaledTargetAbort = 1 << 11,
	ReceivedTargetAbort = 1 << 12,
	ReceivedMasterAbort = 1 << 13,
	SignaledSystemError = 1 << 14,
	DetectedParityError = 1 << 15,
};

inline auto operator|(StatusRegister a, StatusRegister b) -> StatusRegister
{
	return static_cast<StatusRegister>(static_cast<u16>(a) | static_cast<u16>(b));
}

inline auto operator&(StatusRegister a, StatusRegister b) -> StatusRegister
{
	return static_cast<StatusRegister>(static_cast<u16>(a) & static_cast<u16>(b));
}

struct PCIDeviceHeader0 {
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
};

struct DeviceCapability {
	u8 ID;
	u8 Next;
};

struct MSIXCapability : public DeviceCapability {
	u16 MessageControl;
	u32 TableOffset;
	u32 PendingBitOffset;

	static constexpr u8 ID = 0x11;
};

struct MSIXTableEntry {
	u32 AddressLow;
	u32 AddressHigh;
	u32 Data;
	u32 VectorControl;
};

struct PCIDevice {
	auto Init() -> Result<void>;

	auto MapBars() const -> Result<void>;
	[[nodiscard]] auto BarAddress(u8 index) const -> Result<PhysicalAddress>;

	auto EnableMSIX() const -> void;
	auto SetMSIXVector(usize msiVector, u8 systemVector) const -> Result<void>;

	PCIDeviceHeader0* ConfigurationSpace;

	u16 PCISegmentGroupNumber;
	u8 BusNumber;
	u8 DeviceNumber;
	u8 FunctionNumber;

	MSIXCapability* MSIX;
	MSIXTableEntry* MSIXTable;
};

auto ScanPCIDevices() -> Result<void>;

// For now, this is it. In the future when I will actually start using more devices, I will make a proper device list and so on...
extern PCIDevice g_pciStorageDevices[1];

}
