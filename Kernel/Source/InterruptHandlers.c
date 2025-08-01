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
	LogLine(SK_LOG_ERROR "EXCEPTION ENCOUNTERED: %s", exceptionName);
	LogLine(SK_LOG_ERROR "Ring      : %u", frame->CS & 0b11);
	LogLine(SK_LOG_ERROR "Process ID: %u", g_scheduler.CurrentThread->ParentProcess->ID);
	LogLine(SK_LOG_ERROR "Thread ID : %u", g_scheduler.CurrentThread->ID);
	LogLine(SK_LOG_ERROR "");

	LogLine(SK_LOG_ERROR "Interrupt frame:");
	LogLine(SK_LOG_ERROR "RSP   : 0x%x", frame->RSP);
	LogLine(SK_LOG_ERROR "RFLAGS: 0x%x", frame->RFLAGS);
	LogLine(SK_LOG_ERROR "CS    : 0x%x", frame->CS);
	LogLine(SK_LOG_ERROR "SS    : 0x%x", frame->SS);
	LogLine(SK_LOG_ERROR "RIP   : 0x%x", frame->RIP);
	LogLine(SK_LOG_ERROR "");
}

__attribute__((interrupt)) void BreakpointInterruptHandler(InterruptFrame* frame) { PrintCommonExceptionInfo(frame, "Breakpoint"); }

__attribute__((interrupt)) void InvalidOpcodeInterruptHandler(InterruptFrame* frame)
{
	PrintCommonExceptionInfo(frame, "Invalid Opcode");

	if (g_scheduler.CurrentThread->ParentProcess->ID == 0) {
		// Exception occured in the kernel
		LogLine(SK_LOG_ERROR "Kernel is in an unrecoverable state. Hanging...");
		Hang();
	} else {
		// Exception occured in a process
		LogLine(SK_LOG_ERROR "Terminating the faulty process.");
		ProcessRemove(&g_scheduler, g_scheduler.CurrentThread->ParentProcess);
		ScheduleExceptionHandler();
	}
}

__attribute__((interrupt)) void GeneralProtectionFaultInterruptHandler(InterruptFrame* frame, u64 /* unused */)
{
	PrintCommonExceptionInfo(frame, "General Protection Fault");

	if (g_scheduler.CurrentThread->ParentProcess->ID == 0) {
		// Exception occured in the kernel
		LogLine(SK_LOG_ERROR "Kernel is in an unrecoverable state. Hanging...");
		Hang();
	} else {
		// Exception occured in a process
		LogLine(SK_LOG_ERROR "Terminating the faulty process.");
		ProcessRemove(&g_scheduler, g_scheduler.CurrentThread->ParentProcess);
		ScheduleExceptionHandler();
	}
}

__attribute__((interrupt)) void DoubleFaultInterruptHandler(InterruptFrame* frame, u64 /* unused */)
{
	PrintCommonExceptionInfo(frame, "Double Fault");

	if (g_scheduler.CurrentThread->ParentProcess->ID == 0) {
		// Exception occured in the kernel
		LogLine(SK_LOG_ERROR "Kernel is in an unrecoverable state. Hanging...");
		Hang();
	} else {
		// Exception occured in a process
		LogLine(SK_LOG_ERROR "Terminating the faulty process.");
		ProcessRemove(&g_scheduler, g_scheduler.CurrentThread->ParentProcess);
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

	LogLine(SK_LOG_ERROR "Memory info:");
	LogLine(SK_LOG_ERROR "Faulty virtual address: 0x%x", faultVirtualAddress);
	LogLine(SK_LOG_ERROR "PML4 address          : 0x%x", pml4Address);
	LogLine(SK_LOG_ERROR "");

	LogLine(SK_LOG_ERROR "Error code: %u", errorCode);

	if (errorCode & PageFaultCausePresent) {
		LogLine(SK_LOG_ERROR "\tPresent");
	}

	if (errorCode & PageFaultCauseWrite) {
		LogLine(SK_LOG_ERROR "\tWrite");
	} else {
		LogLine(SK_LOG_ERROR "\tRead");
	}

	if (errorCode & PageFaultCauseUser) {
		LogLine(SK_LOG_ERROR "\tUser");
	}

	if (errorCode & PageFaultCauseReservedWrite) {
		LogLine(SK_LOG_ERROR "\tReservedWrite");
	}

	if (errorCode & PageFaultCauseInstructionFetch) {
		LogLine(SK_LOG_ERROR "\tInstructionFetch");
	}

	if (errorCode & PageFaultCauseProtectionKey) {
		LogLine(SK_LOG_ERROR "\tProtectionKey");
	}

	if (errorCode & PageFaultCauseShadowStack) {
		LogLine(SK_LOG_ERROR "\tShadowStack");
	}

	if (errorCode & PageFaultCauseSoftwareGuardExtensions) {
		LogLine(SK_LOG_ERROR "\tSoftwareGuardExtensions");
	}

	if (g_scheduler.CurrentThread->ParentProcess->ID == 0) {
		// Exception occured in the kernel
		LogLine(SK_LOG_ERROR "Kernel is in an unrecoverable state. Hanging...");
		Hang();
	} else {
		// Exception occured in a process
		LogLine(SK_LOG_ERROR "Terminating the faulty process.");
		ProcessRemove(&g_scheduler, g_scheduler.CurrentThread->ParentProcess);
		ScheduleExceptionHandler();
	}
}

__attribute__((interrupt)) void KeyboardInterruptHandler(InterruptFrame* /* unused */)
{
	u8 scanCode = InputU8(0x60);
	i8 character = TranslateScanCode(scanCode);
	if (character != '?') {
		i8 string[2];
		string[0] = character;
		string[1] = '\0';
		Log(string);
	}

	EOISignal();
}
