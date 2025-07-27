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
	Log(SK_LOG_ERROR "EXCEPTION ENCOUNTERED: %s", exceptionName);
	Log(SK_LOG_ERROR "Ring      : %u", frame->CS & 0b11);
	Log(SK_LOG_ERROR "Process ID: %u", g_scheduler.CurrentThread->ParentProcess->ID);
	Log(SK_LOG_ERROR "Thread ID : %u", g_scheduler.CurrentThread->ID);
	Log(SK_LOG_ERROR "");

	Log(SK_LOG_ERROR "Interrupt frame:");
	Log(SK_LOG_ERROR "RSP   : 0x%x", frame->RSP);
	Log(SK_LOG_ERROR "RFLAGS: 0x%x", frame->RFLAGS);
	Log(SK_LOG_ERROR "CS    : 0x%x", frame->CS);
	Log(SK_LOG_ERROR "SS    : 0x%x", frame->SS);
	Log(SK_LOG_ERROR "RIP   : 0x%x", frame->RIP);
	Log(SK_LOG_ERROR "");
}

__attribute__((interrupt)) void BreakpointInterruptHandler(InterruptFrame* frame) { PrintCommonExceptionInfo(frame, "Breakpoint"); }

__attribute__((interrupt)) void InvalidOpcodeInterruptHandler(InterruptFrame* frame)
{
	PrintCommonExceptionInfo(frame, "Invalid Opcode");

	if (g_scheduler.CurrentThread->ParentProcess->ID == 0) {
		// Exception occured in the kernel
		Log(SK_LOG_ERROR "Kernel is in an unrecoverable state. Hanging...");
		Hang();
	} else {
		// Exception occured in a process
		Log(SK_LOG_ERROR "Terminating the faulty process.");
		DeleteProcess(&g_scheduler, g_scheduler.CurrentThread->ParentProcess);
		ScheduleExceptionHandler();
	}
}

__attribute__((interrupt)) void GeneralProtectionFaultInterruptHandler(InterruptFrame* frame, u64 /* unused */)
{
	PrintCommonExceptionInfo(frame, "General Protection Fault");

	if (g_scheduler.CurrentThread->ParentProcess->ID == 0) {
		// Exception occured in the kernel
		Log(SK_LOG_ERROR "Kernel is in an unrecoverable state. Hanging...");
		Hang();
	} else {
		// Exception occured in a process
		Log(SK_LOG_ERROR "Terminating the faulty process.");
		DeleteProcess(&g_scheduler, g_scheduler.CurrentThread->ParentProcess);
		ScheduleExceptionHandler();
	}
}

__attribute__((interrupt)) void DoubleFaultInterruptHandler(InterruptFrame* frame, u64 /* unused */)
{
	PrintCommonExceptionInfo(frame, "Double Fault");

	if (g_scheduler.CurrentThread->ParentProcess->ID == 0) {
		// Exception occured in the kernel
		Log(SK_LOG_ERROR "Kernel is in an unrecoverable state. Hanging...");
		Hang();
	} else {
		// Exception occured in a process
		Log(SK_LOG_ERROR "Terminating the faulty process.");
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

	Log(SK_LOG_ERROR "Memory info:");
	Log(SK_LOG_ERROR "Faulty virtual address: 0x%x", faultVirtualAddress);
	Log(SK_LOG_ERROR "PML4 address          : 0x%x", pml4Address);
	Log(SK_LOG_ERROR "");

	Log(SK_LOG_ERROR "Error code: %u", errorCode);

	if (errorCode & PageFaultCausePresent) {
		Log(SK_LOG_ERROR "\tPresent");
	}

	if (errorCode & PageFaultCauseWrite) {
		Log(SK_LOG_ERROR "\tWrite");
	} else {
		Log(SK_LOG_ERROR "\tRead");
	}

	if (errorCode & PageFaultCauseUser) {
		Log(SK_LOG_ERROR "\tUser");
	}

	if (errorCode & PageFaultCauseReservedWrite) {
		Log(SK_LOG_ERROR "\tReservedWrite");
	}

	if (errorCode & PageFaultCauseInstructionFetch) {
		Log(SK_LOG_ERROR "\tInstructionFetch");
	}

	if (errorCode & PageFaultCauseProtectionKey) {
		Log(SK_LOG_ERROR "\tProtectionKey");
	}

	if (errorCode & PageFaultCauseShadowStack) {
		Log(SK_LOG_ERROR "\tShadowStack");
	}

	if (errorCode & PageFaultCauseSoftwareGuardExtensions) {
		Log(SK_LOG_ERROR "\tSoftwareGuardExtensions");
	}

	if (g_scheduler.CurrentThread->ParentProcess->ID == 0) {
		// Exception occured in the kernel
		Log(SK_LOG_ERROR "Kernel is in an unrecoverable state. Hanging...");
		Hang();
	} else {
		// Exception occured in a process
		Log(SK_LOG_ERROR "Terminating the faulty process.");
		DeleteProcess(&g_scheduler, g_scheduler.CurrentThread->ParentProcess);
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
