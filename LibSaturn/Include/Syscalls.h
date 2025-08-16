#include "Core.h"

/// Implemented in `SyscallWrapper.s`.
u64 SyscallWrapper(usz syscallNumber, u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);

u64 ScPrint(const i8* text);
