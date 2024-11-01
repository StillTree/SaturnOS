#include "Logger.h"

SupernovaLoggerData g_mainLogger = { FALSE, { 0 }, FALSE, { NULL, 0, 0, 0, 0, 0 } };

EFI_STATUS InitLogger(EFI_SYSTEM_TABLE* systemTable, SupernovaLoggerData* logger, BOOLEAN logSerial, BOOLEAN logFramebuffer)
{
	EFI_STATUS status = EFI_SUCCESS;

	logger->logSerial = logSerial;
	logger->logFramebuffer = logFramebuffer;

	if (logSerial) {
		status = InitSerialLogger(systemTable, &logger->serial);
		if (EFI_ERROR(status)) {
			return status;
		}
	}

	if (logFramebuffer) {
		status = InitFramebufferLogger(systemTable, &logger->framebuffer);
		if (EFI_ERROR(status)) {
			return status;
		}
	}

	systemTable->ConOut->ClearScreen(systemTable->ConOut);

	return status;
}

VOID Log(SupernovaLoggerData* logger, SupernovaLogLevel level, CHAR16* message)
{
	if (logger->logSerial) {
		switch (level) {
		case LogDebug:
			SerialLoggerWriteString(&logger->serial, L"[DEBUG]: ");
			break;
		case LogInfo:
			SerialLoggerWriteString(&logger->serial, L"[INFO ]: ");
			break;
		case LogWarn:
			SerialLoggerWriteString(&logger->serial, L"[WARN ]: ");
			break;
		case LogError:
			SerialLoggerWriteString(&logger->serial, L"[ERROR]: ");
			break;
		}

		SerialLoggerWriteString(&logger->serial, message);
		SerialLoggerWriteChar(&logger->serial, L'\n');
	}

	if (logger->logFramebuffer) {
		switch (level) {
		case LogDebug:
			FramebufferLoggerWriteString(&logger->framebuffer, L"[DEBUG]: ");
			break;
		case LogInfo:
			FramebufferLoggerWriteString(&logger->framebuffer, L"[INFO ]: ");
			break;
		case LogWarn:
			FramebufferLoggerWriteString(&logger->framebuffer, L"[WARN ]: ");
			break;
		case LogError:
			FramebufferLoggerWriteString(&logger->framebuffer, L"[ERROR]: ");
			break;
		}

		FramebufferLoggerWriteString(&logger->framebuffer, message);
		FramebufferLoggerWriteChar(&logger->framebuffer, L'\n');
	}
}
