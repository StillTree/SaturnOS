#pragma once

#include "UefiTypes.h"

/// C's memset but without a shitty name.
VOID MemoryFill(VOID* ptr, UINT8 value, UINTN size);

