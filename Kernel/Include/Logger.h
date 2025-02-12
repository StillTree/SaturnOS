#pragma once

#include "Core.h"
#include "Logger/Framebuffer.h"
#include "Logger/SerialConsole.h"

typedef enum LogLevel : u8 { LogLevelDebug, LogLevelInfo, LogLevelWarn, LogLevelError } LogLevel;

typedef struct Logger {
	FramebufferLogger Framebuffer;
	SerialConsoleLogger SerialConsole;
	bool FramebufferEnabled;
	bool SerialConsoleEnabled;
} Logger;

void LoggerInit(Logger* logger, bool framebufferEnabled, bool serialConsoleEnabled, KernelBootInfo* bootInfo, u16 serialConsolePort);
void Log(Logger* logger, LogLevel logLevel, const i8* format, ...);

extern Logger g_mainLogger;

#ifdef SK_DEBUG
#define SK_LOG_DEBUG(...) Log(&g_mainLogger, LogLevelDebug, __VA_ARGS__)
#else
#define SK_LOG_DEBUG(...)
#endif
#define SK_LOG_INFO(...) Log(&g_mainLogger, LogLevelInfo, __VA_ARGS__)
#define SK_LOG_WARN(...) Log(&g_mainLogger, LogLevelWarn, __VA_ARGS__)
#define SK_LOG_ERROR(...) Log(&g_mainLogger, LogLevelError, __VA_ARGS__)
