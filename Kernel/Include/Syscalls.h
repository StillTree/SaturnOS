#pragma once

#include "Core.h"
#include "Memory/VirtualAddress.h"

constexpr u32 MSR_EFER = 0xC0000080;
constexpr u32 MSR_STAR = 0xc0000081;
constexpr u32 MSR_LSTAR = 0xc0000082;
constexpr u32 MSR_SFMASK = 0xc0000084;

Result TestSyscall1();
Result TestSyscall2();

void InitSyscalls();
void SyscallHandler();
void DispatchSyscall(u64 syscallNumber);

extern VirtualAddress g_syscallFunctions[2];
