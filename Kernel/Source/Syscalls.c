#include "Syscalls.h"

#include "Logger.h"
#include "MSR.h"
#include "Memory/VirtualAddress.h"
#include "Result.h"

Result Syscall1()
{
	SK_LOG_DEBUG("Syscall 1");
	return ResultOk;
}

Result Syscall2()
{
	SK_LOG_DEBUG("Syscall 2");
	return ResultOk;
}

VirtualAddress g_syscallFunctions[2] = { (VirtualAddress)Syscall1, (VirtualAddress)Syscall2 };

void InitSyscalls()
{
	u64 efer = ReadMSR(MSR_EFER);
	efer |= 1;
	WriteMSR(MSR_EFER, efer);

	u64 star = (0x8ULL << 32) | (0x13ULL << 48);
	WriteMSR(MSR_STAR, star);

	WriteMSR(MSR_LSTAR, (u64)SyscallHandler);

	WriteMSR(MSR_SFMASK, 0x1ULL << 9);
}
