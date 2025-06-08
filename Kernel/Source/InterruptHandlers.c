#include "InterruptHandlers.h"

#include "APIC.h"
#include "InOut.h"
#include "Keyboard.h"
#include "Logger.h"
#include "Memory/PageTable.h"
#include "Panic.h"

__attribute__((interrupt)) void BreakpointInterruptHandler(InterruptFrame* frame)
{
	SK_LOG_ERROR("EXCEPTION OCCURED: BREAKPOINT, InterruptFrame");
	SK_LOG_ERROR("{");
	SK_LOG_ERROR("\tStackPointer = 0x%x", frame->RSP);
	SK_LOG_ERROR("\tFlags = 0x%x", frame->RFLAGS);
	SK_LOG_ERROR("\tCodeSegment = 0x%x", frame->CS);
	SK_LOG_ERROR("\tSegmentSelector = 0x%x", frame->SS);
	SK_LOG_ERROR("\tInstruction Pointer = 0x%x", frame->RIP);
	SK_LOG_ERROR("}");
}

__attribute__((interrupt)) void InvalidOpcodeInterruptHandler(InterruptFrame* frame)
{
	SK_LOG_ERROR("EXCEPTION OCCURED: INVALID OPCODE, InterruptFrame");
	SK_LOG_ERROR("{");
	SK_LOG_ERROR("\tStackPointer = 0x%x", frame->RSP);
	SK_LOG_ERROR("\tFlags = 0x%x", frame->RFLAGS);
	SK_LOG_ERROR("\tCodeSegment = 0x%x", frame->CS);
	SK_LOG_ERROR("\tSegmentSelector = 0x%x", frame->SS);
	SK_LOG_ERROR("\tInstruction Pointer = 0x%x", frame->RIP);
	SK_LOG_ERROR("}");
}

__attribute__((interrupt)) void GeneralProtectionFaultInterruptHandler(InterruptFrame* frame, u64 /* unused */)
{
	SK_LOG_ERROR("UNRECOVERABLE EXCEPTION OCCURED: GENERAL PROTECTION FAULT, InterruptFrame");
	SK_LOG_ERROR("{");
	SK_LOG_ERROR("\tStackPointer = 0x%x", frame->RSP);
	SK_LOG_ERROR("\tFlags = 0x%x", frame->RFLAGS);
	SK_LOG_ERROR("\tCodeSegment = 0x%x", frame->CS);
	SK_LOG_ERROR("\tSegmentSelector = 0x%x", frame->SS);
	SK_LOG_ERROR("\tInstruction Pointer = 0x%x", frame->RIP);
	SK_LOG_ERROR("}");

	Hang();
}

__attribute__((interrupt)) void DoubleFaultInterruptHandler(InterruptFrame* frame, u64 /* unused */)
{
	SK_LOG_ERROR("UNRECOVERABLE EXCEPTION OCCURED: DOUBLE FAULT, InterruptFrame");
	SK_LOG_ERROR("{");
	SK_LOG_ERROR("\tStackPointer = 0x%x", frame->RSP);
	SK_LOG_ERROR("\tFlags = 0x%x", frame->RFLAGS);
	SK_LOG_ERROR("\tCodeSegment = 0x%x", frame->CS);
	SK_LOG_ERROR("\tSegmentSelector = 0x%x", frame->SS);
	SK_LOG_ERROR("\tInstruction Pointer = 0x%x", frame->RIP);
	SK_LOG_ERROR("}");

	Hang();
}

typedef enum PageFaultCause : u16 {
	PageFaultCausePresent = 1,
	PageFaultCauseWrite = 1 << 1,
	PageFaultCauseUser = 1 << 2,
	PageFaultCauseReservedWrite = 1 << 3,
	PageFaultCauseInstructionFetch = 1 << 4,
	PageFaultCauseProtectionKey = 1 << 5,
	PageFaultCauseShadowStack = 1 << 6,
	PageFaultCauseSoftwareGuardExtensions = 1 << 15,
} PageFaultCause;

__attribute__((interrupt)) void PageFaultInterruptHandler(InterruptFrame* frame, u64 errorCode)
{
	u64 faultVirtualAddress = 0;
	__asm__ volatile("mov %%cr2, %0" : "=r"(faultVirtualAddress));

	u64 pml4Address = KernelPML4();

	SK_LOG_ERROR("UNRECOVERABLE EXCEPTION OCCURED: PAGE FAULT");
	SK_LOG_ERROR("Faulty virtual address = 0x%x", faultVirtualAddress);
	SK_LOG_ERROR("PML4 address = 0x%x", pml4Address);
	SK_LOG_ERROR("Error code = %u, caused by:", errorCode);

	if (errorCode & PageFaultCausePresent) {
		SK_LOG_ERROR("\tPresent");
	}

	if (errorCode & PageFaultCauseWrite) {
		SK_LOG_ERROR("\tWrite");
	} else {
		SK_LOG_ERROR("\tRead");
	}

	if (errorCode & PageFaultCauseUser) {
		SK_LOG_ERROR("\tUser");
	}

	if (errorCode & PageFaultCauseReservedWrite) {
		SK_LOG_ERROR("\tReservedWrite");
	}

	if (errorCode & PageFaultCauseInstructionFetch) {
		SK_LOG_ERROR("\tInstructionFetch");
	}

	if (errorCode & PageFaultCauseProtectionKey) {
		SK_LOG_ERROR("\tProtectionKey");
	}

	if (errorCode & PageFaultCauseShadowStack) {
		SK_LOG_ERROR("\tShadowStack");
	}

	if (errorCode & PageFaultCauseSoftwareGuardExtensions) {
		SK_LOG_ERROR("\tSoftwareGuardExtensions");
	}

	SK_LOG_ERROR("InterruptFrame");
	SK_LOG_ERROR("{");
	SK_LOG_ERROR("\tStackPointer = 0x%x", frame->RSP);
	SK_LOG_ERROR("\tFlags = 0x%x", frame->RFLAGS);
	SK_LOG_ERROR("\tCodeSegment = 0x%x", frame->CS);
	SK_LOG_ERROR("\tSegmentSelector = 0x%x", frame->SS);
	SK_LOG_ERROR("\tInstruction Pointer = 0x%x", frame->RIP);
	SK_LOG_ERROR("}");

	Hang();
}

__attribute__((interrupt)) void KeyboardInterruptHandler(InterruptFrame* /* unused */)
{
	u8 scanCode = InputU8(0x60);
	i8 character = TranslateScanCode(scanCode);
	if (character != '?') {
		FramebufferWriteChar(&g_mainLogger.Framebuffer, character);
		SerialConsoleWriteChar(&g_mainLogger.SerialConsole, character);
	}

	EOISignal();
}
