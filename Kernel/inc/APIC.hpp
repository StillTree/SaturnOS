#include "Core.hpp"
#include "Result.hpp"

namespace SaturnKernel {

constexpr U8 PIC_MASTER_COMMAND = 0x20;
constexpr U8 PIC_MASTER_DATA = 0x21;
constexpr U8 PIC_SLAVE_COMMAND = 0xa0;
constexpr U8 PIC_SLAVE_DATA = 0xa1;

constexpr U32 IA32_APIC_BASE_MSR = 0x1b;

auto InitAPIC() -> Result<void>;

}
