#include "Core.hpp"
#include "Memory/PhysicalAddress.hpp"
#include "Result.hpp"

namespace SaturnKernel {

struct __attribute__((packed)) SDTHeader {
	i8 Signature[4];
	u32 Length;
	u8 Revision;
	u8 Checksum;
	i8 OEMID[6];
	i8 OEMTableID[8];
	u32 OEMRevision;
	u32 CreatorID;
	u32 CreatorRevision;

	[[nodiscard]] auto IsChecksumValid() const -> bool;
};

struct __attribute__((packed)) XSDT : public SDTHeader {
	u64 FirstEntry;

	[[nodiscard]] auto Entries() const -> usize;

	[[nodiscard]] auto GetACPITableAddress(const i8* signature) const -> Result<PhysicalAddress>;
};

struct __attribute__((packed)) MCFG : public SDTHeader {
	struct __attribute__((packed)) Entry {
		u64 BaseAddress;
		u16 SegmentGroupNumber;
		u8 StartBusNumber;
		u8 EndBusNumber;
		u32 Reserved;
	};

	u64 Reserved;

	Entry FirstEntry;

	[[nodiscard]] auto Entries() const -> usize;

	auto GetPCISegmentGroup(usize index) -> Entry*;
};

struct __attribute__((packed)) MADT : public SDTHeader {
	struct __attribute__((packed)) BaseEntry {
		u8 Type;
		u8 Length;
	};

	struct __attribute__((packed)) EntryIO : public BaseEntry {
		u8 IOAPICID;
		u8 Reserved;
		u32 IOAPICAddress;
		u32 GSIBase;
	};

	u32 LocalAPICAddress;
	u32 Flags;
	
	BaseEntry FirstEntry;

	auto GetAPICEntry(BaseEntry*& pointer) -> bool;
};

auto InitXSDT() -> Result<void>;

extern XSDT const* g_xsdt;

}
