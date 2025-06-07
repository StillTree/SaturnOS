#pragma once

#include "Core.h"
#include "Memory/PhysicalAddress.h"

/// Represents a 4 KiB physical memory frame.
typedef u64 Frame4KiB;

constexpr u64 FRAME_4KIB_SIZE_BYTES = 4096;

static inline Frame4KiB Frame4KiBContaining(PhysicalAddress address) { return address & ~0xfff; }
static inline Frame4KiB Frame4KiBNext(PhysicalAddress address) { return (address + FRAME_4KIB_SIZE_BYTES - 1) & ~0xfff; }
