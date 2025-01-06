#include "ACPI.hpp"

#include "Memory.hpp"

namespace SaturnKernel {

XSDT const* g_xsdt = nullptr;

[[nodiscard]] auto SDTHeader::IsChecksumValid() const -> bool
{
	USIZE sum = 0;

	for (USIZE i = 0; i < Length; i++) {
		sum += reinterpret_cast<const I8*>(this)[i];
	}

	return (sum & 0xff) == 0;
}

[[nodiscard]] auto XSDT::Entries() const -> USIZE { return (Header.Length - sizeof(SDTHeader)) / 8; }

[[nodiscard]] auto XSDT::GetACPITableAddress(const I8* signature) const -> Result<PhysicalAddress>
{
	for (USIZE i = 0; i < Entries(); i++) {
		auto* table = PhysicalAddress((&FirstEntry)[i]).AsPointer<SDTHeader>();

		if (table->IsChecksumValid() && MemoryCompare(signature, static_cast<I8*>(table->Signature), 4)) {
			return Result<PhysicalAddress>::MakeOk(PhysicalAddress((&FirstEntry)[i]));
		}
	}

	return Result<PhysicalAddress>::MakeErr(ErrorCode::InvalidSDTSignature);
}

[[nodiscard]] auto MCFG::Entries() const -> USIZE { return (Header.Length - sizeof(SDTHeader) - 8) / sizeof(MCFG::Entry); }

auto MCFG::GetPCISegmentGroup(USIZE index) -> MCFG::Entry { return (&FirstEntry)[index]; }

auto InitXSDT() -> Result<void>
{
	auto* xsdt = PhysicalAddress(g_bootInfo.XSDTAddress).AsPointer<XSDT>();

	if (!xsdt->Header.IsChecksumValid()) {
		return Result<void>::MakeErr(ErrorCode::XSDTCorrupted);
	}

	g_xsdt = xsdt;

	return Result<void>::MakeOk();
}

}
