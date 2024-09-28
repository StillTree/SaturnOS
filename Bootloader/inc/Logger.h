#pragma once

#include "Logger/Framebuffer.h"
#include "Logger/Serial.h"
#include "UefiTypes.h"

/// Wrapper state around the two logger outputs.
typedef struct SupernovaLoggerData
{
	BOOLEAN logSerial;
	SerialLoggerData serial;
	BOOLEAN logFramebuffer;
	FramebufferLoggerData framebuffer;
} SupernovaLoggerData;

/// The log level.
typedef enum SupernovaLogLevel
{
	LogDebug,
	LogInfo,
	LogWarn,
	LogError
} SupernovaLogLevel;

/// Initializes the logger outputs based on the provided BOOLEANs
/// and initializes the wrapper logger state for later use.
EFI_STATUS InitLogger(EFI_SYSTEM_TABLE* systemTable, SupernovaLoggerData* logger, BOOLEAN logSerial, BOOLEAN logFramebuffer);
/// Logs the provided message to the enabled logger outputs.
VOID Log(SupernovaLoggerData* logger, SupernovaLogLevel level, CHAR16* message);

/// The global logger state that should be initialized with empty values
/// and then initialized with the InitLogger function.
extern SupernovaLoggerData g_mainLogger;

#define SN_LOG_DEBUG(message) Log(&g_mainLogger, LogDebug, message)
#define SN_LOG_INFO(message)  Log(&g_mainLogger, LogInfo, message)
#define SN_LOG_WARN(message)  Log(&g_mainLogger, LogWarn, message)
#define SN_LOG_ERROR(message) Log(&g_mainLogger, LogError, message)
