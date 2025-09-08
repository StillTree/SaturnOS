#pragma once

#include "Core.h"

typedef u64 PhysAddr;

static inline void* PhysAddrAsPointer(PhysAddr address) { return (void*)(address + g_bootInfo.PhysicalMemoryOffset); }
