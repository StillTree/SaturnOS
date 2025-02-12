#pragma once

#include "Core.h"
#include "Result.h"

typedef struct SerialConsoleLogger {
	u16 Port;
} SerialConsoleLogger;

Result SerialConsoleInit(SerialConsoleLogger* logger, u16 port);
void SerialConsoleWriteChar(SerialConsoleLogger* logger, u8 character);
void SerialConsoleWriteString(SerialConsoleLogger* logger, const i8* string);
