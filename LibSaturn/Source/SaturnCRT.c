#include "Core.h"
#include "Syscalls.h"

extern u64 Main();

void Start()
{
	// For now return value ignored
	// TODO: Return value support
	Main();

	SyscallWrapper(SYSCALL_PROCESS_TERMINATE, 0, 0, 0, 0, 0, 0);
}
