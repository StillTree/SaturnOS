#include "Core.hpp"
#include "Result.hpp"

namespace SaturnKernel {

constexpr U8 PIC_MASTER_COMMAND = 0x20;
constexpr U8 PIC_MASTER_DATA = 0x21;
constexpr U8 PIC_SLAVE_COMMAND = 0xa0;
constexpr U8 PIC_SLAVE_DATA = 0xa1;

namespace LAPIC {
	constexpr U32 BASE_MSR = 0x1b;
	constexpr U32 SVR_REGISTER_MSR = 0x80f;
}

namespace IOAPIC {
	constexpr U32 ID_REGISTER_INDEX = 0x0;
	constexpr U32 VERSION_REGISTER_INDEX = 0x2;
	constexpr U32 ARBITRATION_REGISTER_INDEX = 0x3;
	constexpr U32 REDIRECTION_TABLE_REGISTER_INDEX = 0x10;

	auto ReadRegister(U32 reg) -> U32;
	auto WriteRegister(U32 reg, U32 value) -> void;
}

auto DisablePIC() -> void;
auto InitAPIC() -> Result<void>;
auto EOISignal() -> void;

}
