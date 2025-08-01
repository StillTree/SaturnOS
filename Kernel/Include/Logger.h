#pragma once

#include "Core.h"
#include "Logger/Framebuffer.h"
#include "Logger/SerialConsole.h"

typedef struct Logger {
	FramebufferLogger Framebuffer;
	SerialConsoleLogger SerialConsole;
	bool FramebufferEnabled;
	bool SerialConsoleEnabled;
} Logger;

void LoggerInit(bool framebufferEnabled, bool serialConsoleEnabled, KernelBootInfo* bootInfo, u16 serialConsolePort);
void LogLine(const i8* format, ...);
void Log(const i8* format, ...);

#define SK_LOG_DEBUG "[DEBUG]: "
#define SK_LOG_INFO "[INFO ]: "
#define SK_LOG_WARN "[WARN ]: "
#define SK_LOG_ERROR "[ERROR]: "
