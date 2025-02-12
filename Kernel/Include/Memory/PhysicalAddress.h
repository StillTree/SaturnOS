#pragma once

#include "Core.h"

typedef u64 PhysicalAddress;

static inline void* PhysicalAddressAsPointer(PhysicalAddress address) { return (void*)(address + g_bootInfo.PhysicalMemoryOffset); }
