#include "Core.hpp"
#include "Memory/PhysicalAddress.hpp"
#include "Result.hpp"

namespace SaturnKernel {

struct __attribute__((packed)) SDTHeader {
	I8 Signature[4];
	U32 Length;
	U8 Revision;
	U8 Checksum;
	I8 OEMID[6];
	I8 OEMTableID[8];
	U32 OEMRevision;
	U32 CreatorID;
	U32 CreatorRevision;

	[[nodiscard]] auto IsChecksumValid() const -> bool;
};

struct __attribute__((packed)) XSDT {
	SDTHeader Header;

	U64 FirstEntry;

	[[nodiscard]] auto Entries() const -> USIZE;

	[[nodiscard]] auto GetACPITableAddress(const I8* signature) const -> Result<PhysicalAddress>;
};

struct __attribute__((packed)) MCFG {
	struct __attribute__((packed)) Entry {
		U64 BaseAddress;
		U16 SegmentGroupNumber;
		U8 StartBusNumber;
		U8 EndBusNumber;
		U32 Reserved;
	};

	SDTHeader Header;

	U64 Reserved;

	Entry FirstEntry;

	[[nodiscard]] auto Entries() const -> USIZE;

	auto GetPCISegmentGroup(USIZE index) -> Entry;
};

auto InitXSDT() -> Result<void>;

extern XSDT const* g_xsdt;

}
