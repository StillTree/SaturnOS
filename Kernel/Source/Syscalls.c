#include "Syscalls.h"

#include "Logger.h"
#include "MSR.h"
#include "Memory/VirtualAddress.h"
#include "Result.h"

VirtualAddress g_syscallFunctions[2] = { (VirtualAddress)TestSyscall1, (VirtualAddress)TestSyscall2 };

Result TestSyscall1()
{
	SK_LOG_DEBUG("Syscall 1");
	return ResultOk;
}

Result TestSyscall2()
{
	SK_LOG_DEBUG("Syscall 2");
	return ResultOk;
}

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
