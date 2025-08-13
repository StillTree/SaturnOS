#pragma once

#include "Core.h"
#include "Memory/VirtualAddress.h"

constexpr u32 MSR_EFER = 0xc0000080;
constexpr u32 MSR_STAR = 0xc0000081;
constexpr u32 MSR_LSTAR = 0xc0000082;
constexpr u32 MSR_SFMASK = 0xc0000084;

/// Syscall number 0.
/// When passing in the ID of 0, the calling process will get terminated. This is entirely done in the assembly handler.
Result ScProcessTerminate(usz processID);
Result ScTest();

void InitSyscalls();
void SyscallHandler();
void DispatchSyscall(u64 syscallNumber);

extern VirtualAddress g_syscallFunctions[2];
