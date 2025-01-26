#pragma once

#include "Core.h"

typedef u64 PhysicalAddress;

inline void* PhysicalAddressAsPointer(PhysicalAddress address) { return (void*)(address + g_bootInfo.PhysicalMemoryOffset); }
