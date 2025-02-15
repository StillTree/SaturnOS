#include "Core.h"
#include "Memory/PhysicalAddress.h"
#include "Result.h"

typedef struct __attribute__((packed)) SDTHeader {
	i8 Signature[4];
	u32 Length;
	u8 Revision;
	u8 Checksum;
	i8 OEMID[6];
	i8 OEMTableID[8];
	u32 OEMRevision;
	u32 CreatorID;
	u32 CreatorRevision;
} SDTHeader;

bool SDTIsChecksumValid(const SDTHeader* acpiTable);

typedef struct __attribute__((packed)) XSDT {
	SDTHeader Header;

	u64 Entries[];
} XSDT;

Result GetACPITableAddress(const i8* signature, PhysicalAddress* address);

typedef struct __attribute__((packed)) MCFGEntry {
	u64 BaseAddress;
	u16 SegmentGroupNumber;
	u8 StartBusNumber;
	u8 EndBusNumber;
	u32 Reserved;
} MCFGEntry;

typedef struct __attribute__((packed)) MCFG {
	SDTHeader Header;

	u64 Reserved;

	MCFGEntry Entries[];
} MCFG;

usz MCFGEntries(const MCFG* mcfg);

MCFGEntry* MCFGGetPCISegmentGroup(MCFG* mcfg, usz index);

typedef enum MADTEntryType : u8 {
	MADTEntryLocalAPIC = 0,
	MADTEntryIOAPIC = 1,
	MADTEntryIOAPICInterruptSourceOverride = 2,
	MADTEntryIOAPICNMISource = 3,
	MADTEntryLocalAPICNMI = 4,
	MADTEntryLocalAPICAddressOverride = 5,
	MADTEntryLocalX2APIC = 9,
} MADTEntryType;

typedef struct __attribute__((packed)) MADTBaseEntry {
	u8 Type;
	u8 Length;
} MADTBaseEntry;

typedef struct __attribute__((packed)) MADTEntryIO {
	MADTBaseEntry Base;
	u8 IOAPICID;
	u8 Reserved;
	u32 IOAPICAddress;
	u32 GSIBase;
} MADTEntryIO;

typedef struct __attribute__((packed)) MADT {
	SDTHeader Header;

	u32 LocalAPICAddress;
	u32 Flags;

	MADTBaseEntry Entries[];
} MADT;

u8 MADTGetAPICEntry(const MADT* madt, MADTBaseEntry** pointer);

Result InitXSDT();

extern XSDT* g_xsdt;
