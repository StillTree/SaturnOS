#include "Core.h"
#include "Result.h"

constexpr u8 PIC_MASTER_COMMAND = 0x20;
constexpr u8 PIC_MASTER_DATA = 0x21;
constexpr u8 PIC_SLAVE_COMMAND = 0xa0;
constexpr u8 PIC_SLAVE_DATA = 0xa1;

constexpr u32 APIC_BASE_MSR = 0x1b;
constexpr u32 X2APIC_MSR_BASE = 0x800;

constexpr u32 LAPIC_ID_REGISTER = 0x20;
constexpr u32 LAPIC_SVR_REGISTER = 0xf0;
constexpr u32 LAPIC_EOI_REGISTER = 0xb0;

constexpr u32 IOAPIC_ID_REGISTER_INDEX = 0x0;
constexpr u32 IOAPIC_VERSION_REGISTER_INDEX = 0x2;
constexpr u32 IOAPIC_ARBITRATION_REGISTER_INDEX = 0x3;
constexpr u32 IOAPIC_REDIRECTION_TABLE_REGISTER_INDEX = 0x10;

u32 LAPICReadRegister(u32 reg);
void LAPICWriteRegister(u32 reg, u32 value);

u32 IOAPICReadRegister(u32 reg);
void IOAPICWriteRegister(u32 reg, u32 value);

void IOAPICSetRedirectionEntry(u8 irq, u64 entry);

typedef struct APIC {
	/// `true` - x2APIC, `false` - xAPIC.
	bool X2APICMode;
	u32* IOAPICAddress;
	/// Unused when in x2APIC mode.
	u32* XAPICAddress;
} APIC;

void DisablePIC();
Result InitAPIC();
void EOISignal();

extern APIC g_apic;
