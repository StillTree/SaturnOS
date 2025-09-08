#pragma once

#include "Core.h"
#include "Memory/PhysAddr.h"

/// Represents a 4 KiB physical memory frame.
typedef u64 Frame4KiB;

constexpr u64 FRAME_4KIB_SIZE_BYTES = 4096;

static inline Frame4KiB Frame4KiBContaining(PhysAddr address) { return __builtin_align_down(address, FRAME_4KIB_SIZE_BYTES); }
static inline Frame4KiB Frame4KiBNext(PhysAddr address) { return __builtin_align_up(address, FRAME_4KIB_SIZE_BYTES); }
static inline bool Frame4KiBAlignCheck(PhysAddr address) { return __builtin_is_aligned(address, FRAME_4KIB_SIZE_BYTES); }
