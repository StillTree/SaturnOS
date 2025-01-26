#pragma once

#include "Core.h"

typedef enum Result : u8 {
	ResultOk = 0,
	ResultNotEnoughMemoryPages,
	ResultNotEnoughMemoryFrames,
	ResultSerialOutputUnavailabe,
	ResultOutOfMemory,
	ResultFrameAlreadyDeallocated,
	ResultPageAlreadyMapped,
	ResultPageAlreadyUnmapped,
	ResultHeapBlockTooSmall,
	ResultHeapBlockIncorrectAlignment,
	ResultHeapBlockIncorrectSplitSize,
	ResultInvalidSDTSignature,
	ResultXSDTCorrupted,
	ResultX2APICUnsupported,
	ResultIOAPICNotPresent,
	ResultInvalidBARIndex,
	ResultInvalidMSIXVector,
	ResultPCICapabilitiesNotSupported,
} Result;
