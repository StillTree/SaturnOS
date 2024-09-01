#pragma once

#include "UefiTypes.h"
#include "Logger/Serial.h"
#include "Logger/Framebuffer.h"

typedef struct SupernovaLoggerData
{
	BOOLEAN               logSerial;
	SerialLoggerData      serial;
	BOOLEAN               logFramebuffer;
	FramebufferLoggerData framebuffer;
} SupernovaLoggerData;

typedef enum SupernovaLogLevel
{
	LogDebug,
	LogInfo,
	LogWarn,
	LogError
} SupernovaLogLevel;

EFI_STATUS InitLogger(EFI_SYSTEM_TABLE* systemTable, SupernovaLoggerData* logger, BOOLEAN logSerial, BOOLEAN logFramebuffer);
VOID Log(SupernovaLoggerData* logger, SupernovaLogLevel level, CHAR16* message);

extern SupernovaLoggerData g_mainLogger;

#define SN_LOG_DEBUG(message) Log(&g_mainLogger, LogDebug, message)
#define SN_LOG_INFO(message) Log(&g_mainLogger, LogInfo, message)
#define SN_LOG_WARN(message) Log(&g_mainLogger, LogWarn, message)
#define SN_LOG_ERROR(message) Log(&g_mainLogger, LogError, message)

