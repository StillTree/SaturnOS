#include "Core.hpp"
#include "Result.hpp"

namespace SaturnKernel {

constexpr u8 PIC_MASTER_COMMAND = 0x20;
constexpr u8 PIC_MASTER_DATA = 0x21;
constexpr u8 PIC_SLAVE_COMMAND = 0xa0;
constexpr u8 PIC_SLAVE_DATA = 0xa1;

namespace LAPIC {
	constexpr u32 BASE_MSR = 0x1b;
	constexpr u32 ID_REGISTER_MSR = 0x802;
	constexpr u32 SVR_REGISTER_MSR = 0x80f;
}

namespace IOAPIC {
	constexpr u32 ID_REGISTER_INDEX = 0x0;
	constexpr u32 VERSION_REGISTER_INDEX = 0x2;
	constexpr u32 ARBITRATION_REGISTER_INDEX = 0x3;
	constexpr u32 REDIRECTION_TABLE_REGISTER_INDEX = 0x10;

	auto ReadRegister(u32 reg) -> u32;
	auto WriteRegister(u32 reg, u32 value) -> void;

	auto SetRedirectionEntry(u8 irq, u64 entry);
}

auto DisablePIC() -> void;
auto InitAPIC() -> Result<void>;
auto EOISignal() -> void;

}
