#include "Syscalls.h"

u64 ScPrint(const i8* text)
{
	return SyscallWrapper(SYSCALL_PRINT, (u64)text, 0, 0, 0, 0, 0);
}
