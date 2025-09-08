#include "Syscalls.h"

#include "GDT.h"
#include "Scheduler.h"
#include "Logger.h"
#include "MSR.h"
#include "Memory/VirtualAddress.h"
#include "Panic.h"
#include "Result.h"

VirtualAddress g_syscallFunctions[3] = { (VirtualAddress)ScProcessTerminate, (VirtualAddress)ScTest, (VirtualAddress)ScPrint };

void ScProcessTerminate(usz processID)
{
	if (processID != 0) {
		LogLine(SK_LOG_DEBUG "Terminating process %u", processID);
		SK_PANIC("Terminating other processes is not implemented yet.");
	}

	// The calling process is terminating itself
	ScheduleProcessTerminate();
}

Result ScTest()
{
	LogLine(SK_LOG_DEBUG "Test syscall");
	return ResultOk;
}

Result ScPrint(const i8* text)
{
	Log(text);

	return ResultOk;
}

void InitSyscalls()
{
	u64 efer = ReadMSR(MSR_EFER);
	efer |= 1;
	WriteMSR(MSR_EFER, efer);

	u64 star = ((u64)GDT_ENTRY_KERNEL_CODE << 32) | (0x13ULL << 48);
	WriteMSR(MSR_STAR, star);

	WriteMSR(MSR_LSTAR, (u64)SyscallHandler);

	WriteMSR(MSR_SFMASK, 1ULL << 9);
}
