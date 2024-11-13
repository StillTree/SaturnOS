#include "Memory/PageTable.hpp"

namespace SaturnKernel {

[[nodiscard]] auto PageTableEntry::Flags() const -> PageTableEntryFlags { return static_cast<PageTableEntryFlags>(Entry); }

[[nodiscard]] auto PageTableEntry::PhysicalFrameAddress() const -> U64 { return ((Entry >> 12) & FRAME_ADDRESS_MASK) << 12; }

auto PageTableEntry::Flags(PageTableEntryFlags flags) -> void { Entry |= static_cast<U64>(flags); }

auto PageTableEntry::PhysicalFrameAddress(U64 address) -> void { Entry |= address; }

}
