#include "Syscalls.h"

#include "GDT.h"
#include "Logger.h"
#include "MSR.h"
#include "Memory/VirtualAddress.h"
#include "Result.h"

VirtualAddress g_syscallFunctions[2] = { (VirtualAddress)ScProcessTerminate, (VirtualAddress)ScTest };

Result ScProcessTerminate(usz processID)
{
	LogLine(SK_LOG_DEBUG "Terminating process %u", processID);
	LogLine(SK_LOG_WARN "This syscall doesn't work ;D");

	return ResultSerialOutputUnavailable;
}

Result ScTest()
{
	LogLine(SK_LOG_DEBUG "Test syscall");
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

	WriteMSR(MSR_SFMASK, 0x1ULL << 9);
}
