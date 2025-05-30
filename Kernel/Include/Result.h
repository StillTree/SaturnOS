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
	ResultInvalidSDTSignature,
	ResultXSDTCorrupted,
	ResultX2APICUnsupported,
	ResultIOAPICNotPresent,
	ResultInvalidBARIndex,
	ResultInvalidMSIXVector,
	ResultPCICapabilitiesUnsupported,
	ResultTimeout,
	ResultOutOfRange,
	ResultAHCI64BitAddressingUnsupported,
	ResultAHCIDeviceUnsupportedSectorSize,
	ResultInvalidPath,
	ResultNotFound,
	ResultInvalidProcessID,
	ResultInvalidPageAlignment,
	ResultInvalidFrameAlignment,
} Result;
