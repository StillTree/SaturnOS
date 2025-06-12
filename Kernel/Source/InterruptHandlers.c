#include "InterruptHandlers.h"

#include "APIC.h"
#include "InOut.h"
#include "Keyboard.h"
#include "Logger.h"
#include "Memory/PageTable.h"
#include "Panic.h"
#include "Scheduler.h"

static void PrintCommonExceptionInfo(InterruptFrame* frame, const i8* exceptionName)
{
	SK_LOG_ERROR("EXCEPTION ENCOUNTERED: %s", exceptionName);
	SK_LOG_ERROR("Ring      : %u", frame->CS & 0b11);
	SK_LOG_ERROR("Process ID: %u", g_scheduler.CurrentThread->ParentProcess->ID);
	SK_LOG_ERROR("Thread ID : %u", g_scheduler.CurrentThread->ID);
	SK_LOG_ERROR("");

	SK_LOG_ERROR("Interrupt frame:");
	SK_LOG_ERROR("RSP   : 0x%x", frame->RSP);
	SK_LOG_ERROR("RFLAGS: 0x%x", frame->RFLAGS);
	SK_LOG_ERROR("CS    : 0x%x", frame->CS);
	SK_LOG_ERROR("SS    : 0x%x", frame->SS);
	SK_LOG_ERROR("RIP   : 0x%x", frame->RIP);
	SK_LOG_ERROR("");
}

__attribute__((interrupt)) void BreakpointInterruptHandler(InterruptFrame* frame) { PrintCommonExceptionInfo(frame, "Breakpoint"); }

__attribute__((interrupt)) void InvalidOpcodeInterruptHandler(InterruptFrame* frame)
{
	PrintCommonExceptionInfo(frame, "Invalid Opcode");

	if (g_scheduler.CurrentThread->ParentProcess->ID == 0) {
		// Exception occured in the kernel
		SK_LOG_ERROR("Kernel is in an unrecoverable state. Hanging...");
		Hang();
	} else {
		// Exception occured in a process
		SK_LOG_ERROR("Terminating the faulty process.");
		DeleteProcess(&g_scheduler, g_scheduler.CurrentThread->ParentProcess);
		ScheduleExceptionHandler();
	}
}

__attribute__((interrupt)) void GeneralProtectionFaultInterruptHandler(InterruptFrame* frame, u64 /* unused */)
{
	PrintCommonExceptionInfo(frame, "General Protection Fault");

	if (g_scheduler.CurrentThread->ParentProcess->ID == 0) {
		// Exception occured in the kernel
		SK_LOG_ERROR("Kernel is in an unrecoverable state. Hanging...");
		Hang();
	} else {
		// Exception occured in a process
		SK_LOG_ERROR("Terminating the faulty process.");
		DeleteProcess(&g_scheduler, g_scheduler.CurrentThread->ParentProcess);
		ScheduleExceptionHandler();
	}
}

__attribute__((interrupt)) void DoubleFaultInterruptHandler(InterruptFrame* frame, u64 /* unused */)
{
	PrintCommonExceptionInfo(frame, "Double Fault");

	if (g_scheduler.CurrentThread->ParentProcess->ID == 0) {
		// Exception occured in the kernel
		SK_LOG_ERROR("Kernel is in an unrecoverable state. Hanging...");
		Hang();
	} else {
		// Exception occured in a process
		SK_LOG_ERROR("Terminating the faulty process.");
		DeleteProcess(&g_scheduler, g_scheduler.CurrentThread->ParentProcess);
		ScheduleExceptionHandler();
	}
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
	PrintCommonExceptionInfo(frame, "Page Fault");

	u64 faultVirtualAddress = 0;
	__asm__ volatile("mov %%cr2, %0" : "=r"(faultVirtualAddress));

	u64 pml4Address = KernelPML4();

	SK_LOG_ERROR("Memory info:");
	SK_LOG_ERROR("Faulty virtual address: 0x%x", faultVirtualAddress);
	SK_LOG_ERROR("PML4 address          : 0x%x", pml4Address);
	SK_LOG_ERROR("");

	SK_LOG_ERROR("Error code: %u", errorCode);

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

	if (g_scheduler.CurrentThread->ParentProcess->ID == 0) {
		// Exception occured in the kernel
		SK_LOG_ERROR("Kernel is in an unrecoverable state. Hanging...");
		Hang();
	} else {
		// Exception occured in a process
		SK_LOG_ERROR("Terminating the faulty process.");
		DeleteProcess(&g_scheduler, g_scheduler.CurrentThread->ParentProcess);
		ScheduleExceptionHandler();
	}
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
