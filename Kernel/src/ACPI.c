#include "ACPI.h"

#include "Memory.h"

XSDT const* g_xsdt = nullptr;

[[nodiscard]] auto SDTHeader::IsChecksumValid() const -> bool
{
	usize sum = 0;

	for (usize i = 0; i < Length; i++) {
		sum += reinterpret_cast<const i8*>(this)[i];
	}

	return (sum & 0xff) == 0;
}

[[nodiscard]] auto XSDT::Entries() const -> usize { return (Length - sizeof(SDTHeader)) / 8; }

[[nodiscard]] auto XSDT::GetACPITableAddress(const i8* signature) const -> Result<PhysicalAddress>
{
	for (usize i = 0; i < Entries(); i++) {
		auto* table = PhysicalAddress((&FirstEntry)[i]).AsPointer<SDTHeader>();

		if (table->IsChecksumValid() && MemoryCompare(signature, static_cast<i8*>(table->Signature), 4)) {
			return Result<PhysicalAddress>::MakeOk(PhysicalAddress((&FirstEntry)[i]));
		}
	}

	return Result<PhysicalAddress>::MakeErr(ErrorCode::InvalidSDTSignature);
}

[[nodiscard]] auto MCFG::Entries() const -> usize { return (Length - sizeof(SDTHeader) - 8) / sizeof(MCFG::Entry); }

auto MCFG::GetPCISegmentGroup(usize index) -> MCFG::Entry* { return &(&FirstEntry)[index]; }

auto MADT::GetAPICEntry(BaseEntry*& pointer) -> bool
{
	u8* offset = reinterpret_cast<u8*>(pointer);

	if(offset + pointer->Length >= reinterpret_cast<u8*>(this) + Length) {
		return false;
	}

	offset += pointer->Length;
	pointer = reinterpret_cast<BaseEntry*>(offset);

	return true;
}

auto InitXSDT() -> Result<void>
{
	auto* xsdt = PhysicalAddress(g_bootInfo.XSDTAddress).AsPointer<XSDT>();

	if (!xsdt->IsChecksumValid()) {
		return Result<void>::MakeErr(ErrorCode::XSDTCorrupted);
	}

	g_xsdt = xsdt;

	return Result<void>::MakeOk();
}
