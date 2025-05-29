#pragma once

#include "Core.h"
#include "Memory/PhysicalAddress.h"
#include "Result.h"

constexpr u32 HBA_PORT_DEVICE_PRESENT = 0x3;

typedef enum ATACommand : u8 {
	ATACommandReadDMAExtended = 0x25,
	ATACommandIdentifyDevice = 0xec,
	ATACommandWriteDMAExtended = 0x35
} ATACommand;

typedef enum FISType : u8 {
	FISTypeRegisterH2D = 0x27
} FISType;

typedef struct FISRegisterH2D {
	FISType FISType;
	u8 PortMultiplier;
	ATACommand Command;
	u8 FeatureLow;

	u8 LBA0;
	u8 LBA1;
	u8 LBA2;
	u8 Device;

	u8 LBA3;
	u8 LBA4;
	u8 LBA5;
	u8 FeatureHigh;

	u16 Count;
	u8 IsochronousCommandCompletion;
	u8 Control;

	u8 Reserved[4];
} FISRegisterH2D;

constexpr u16 FIS_DWORD_LENGTH_REGISTER_H2D = sizeof(FISRegisterH2D) / sizeof(u32);

typedef struct HBAPortRegisters {
	/// Command List Base Address
	u32 CLB;
	/// Command List Base Address Upper 32-Bits
	u32 CLBU;
	/// FIS Base Address
	u32 FB;
	/// FIS Base Address Upper 32-Bits
	u32 FBU;
	/// Interrupt Status
	u32 IS;
	/// Interrupt Enable
	u32 IE;
	/// Command and Status
	u32 CMD;
	u32 Reserved1;
	/// Task File Data
	u32 TFD;
	/// Signature
	u32 SIG;
	/// Serial ATA Status
	u32 SSTS;
	/// Serial ATA Control
	u32 SCTL;
	/// Serial ATA Error
	u32 SERR;
	/// Serial ATA Active
	u32 SACT;
	/// Command Issue
	u32 CI;
	/// Serial ATA Notification
	u32 SNTF;
	/// FIS-based Switching Control
	u32 FBS;
	/// Device Sleep
	u32 DEVSLP;
	u32 Reserved2[11];
	u32 VendorSpecific[4];
} HBAPortRegisters;

typedef struct HBARegisters {
	/// HBA Capabilities
	u32 CAP;
	/// Global HBA Control
	u32 GHC;
	/// Interrupt Status
	u32 IS;
	/// Ports Implemented
	u32 PI;
	/// AHCI Version
	u32 VS;
	/// Command Completion Coalescing Control
	u32 CCCCTL;
	/// Command Completion Coalescing Ports
	u32 CCCPORTS;
	/// Enclosure Management Location
	u32 EMLOC;
	/// Enclosure Management Control
	u32 EMCTL;
	/// Capabilities Extended
	u32 CAP2;
	/// BIOS/OS Handoff Control and Status
	u32 BHOC;

	u8 Reserved[116];
	u8 VendorSpecific[96];

	HBAPortRegisters Ports[32];
} HBARegisters;

typedef enum AHCICommandHeaderParameters : u16 {
	AHCICommandATAPI = 1 << 5,
	AHCICommandWrite = 1 << 6,
	AHCICommandPrefetchable = 1 << 7,
	AHCICommandReset = 1 << 8,
	AHCICommandBIST = 1 << 9,
	AHCICommandClearBusy = 1 << 10
} AHCICommandHeaderParameters;

typedef struct AHCICommandHeader {
	/// Command FIS Length, ATAPI, Write, Prefetchable, Reset, BIST, Clear Busy upon R_OK, Port Multiplier Port
	u16 CFL;
	/// Physical Region Descriptor Table Length
	u16 PRDTL;
	/// Physical Region Descriptor Byte Count
	u32 PRDBC;
	/// Command Table Descriptor Base Address
	u32 CTBA;
	/// Command Table Descriptor Base Address Upper 32-bits
	u32 CTBAU;
	u32 Reserved[4];
} AHCICommandHeader;

typedef struct AHCIPRDT {
	/// Data Base Address
	u32 DBA;
	/// Data Base Address Upper 32-bits
	u32 DBAU;
	u32 Reserved;
	/// Data Byte Count, Interrupt on Completion
	u32 DBC;
} AHCIPRDT;

typedef struct AHCICommandTable {
	/// Command FIS
	u8 CFIS[64];
	/// ATAPI Command
	u8 ACMD[16];
	u8 Reserved[48];
	/// Physical Region Descriptor Table
	AHCIPRDT PRDT[];
} AHCICommandTable;

typedef struct AHCIDevice {
	usz SectorSize;
	usz SectorCount;
	u8 Port;

	HBAPortRegisters* Registers;
	AHCICommandHeader* CommandList;
} AHCIDevice;

Result AHCIDeviceInit(AHCIDevice* device);
Result AHCIDeviceReadSectors(AHCIDevice* device, usz startSectorIndex, u16 sectorCount, PhysicalAddress buffer);
Result AHCIDeviceWriteSectors(AHCIDevice* device, usz startSectorIndex, usz sectorCount, PhysicalAddress buffer);

typedef struct AHCIDriver {
	/// PCI BAR 5
	HBARegisters* Registers;

	/// For now I only store one device (port), later I will implement multiple device support.
	AHCIDevice Devices[1];
} AHCIDriver;

Result AHCIReset(AHCIDriver* ahci);

Result InitAHCI();

extern AHCIDriver g_ahciDriver;
