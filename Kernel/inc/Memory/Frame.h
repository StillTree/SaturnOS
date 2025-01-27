#pragma once

#include "Core.h"
#include "Memory/PhysicalAddress.h"

/// Represents a 4 KiB physical memory frame.
typedef u64 Frame4KiB;

#define FRAME_4KIB_SIZE_BYTES 4096

static inline Frame4KiB Frame4KiBContainingAddress(PhysicalAddress address) { return address & ~0xfff; }
