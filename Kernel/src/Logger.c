#include "Logger.h"

#include "Format.h"
#include <stdarg.h>

Logger g_mainLogger;

void LoggerInit(Logger* logger, bool framebufferEnabled, bool serialConsoleEnabled, KernelBootInfo* bootInfo, u16 serialConsolePort)
{
	logger->FramebufferEnabled = framebufferEnabled;
	logger->SerialConsoleEnabled = serialConsoleEnabled;

	if (framebufferEnabled) {
		FramebufferLogger framebufferLogger = { .Framebuffer = bootInfo->Framebuffer,
			.FramebufferSize = bootInfo->FramebufferSize,
			.Width = bootInfo->FramebufferWidth,
			.Height = bootInfo->FramebufferHeight,
			.CursorPositionX = 0,
			.CursorPositionY = 0 };
		logger->Framebuffer = framebufferLogger;

		FramebufferClear(&logger->Framebuffer);
	}

	if (serialConsoleEnabled) {
		Result result = SerialConsoleInit(&logger->SerialConsole, serialConsolePort);
		if (result) {
			SK_LOG_WARN("The serial output device at port 0x3f8 is not functioning correctly");
			logger->SerialConsoleEnabled = false;
		}
	}
}

void Log(Logger* logger, LogLevel logLevel, const i8* format, ...)
{
	if (logger->FramebufferEnabled) {
		switch (logLevel) {
		case LogLevelDebug:
			FramebufferWriteString(&logger->Framebuffer, "[DEBUG]: ");
			break;
		case LogLevelInfo:
			FramebufferWriteString(&logger->Framebuffer, "[INFO ]: ");
			break;
		case LogLevelWarn:
			FramebufferWriteString(&logger->Framebuffer, "[WARN ]: ");
			break;
		case LogLevelError:
			FramebufferWriteString(&logger->Framebuffer, "[ERROR]: ");
			break;
		}
	}

	if (logger->SerialConsoleEnabled) {
		switch (logLevel) {
		case LogLevelDebug:
			SerialConsoleWriteString(&logger->SerialConsole, "[DEBUG]: ");
			break;
		case LogLevelInfo:
			SerialConsoleWriteString(&logger->SerialConsole, "[INFO ]: ");
			break;
		case LogLevelWarn:
			SerialConsoleWriteString(&logger->SerialConsole, "[WARN ]: ");
			break;
		case LogLevelError:
			SerialConsoleWriteString(&logger->SerialConsole, "[ERROR]: ");
			break;
		}
	}

	va_list args;
	va_start(args, format);

	usz i = 0;
	while (format[i] != '\0') {
		if (format[i] == '{' && format[i + 1] == '}') {
			i8 hexOutput[MAX_HEX_LENGTH];
			u64 number = va_arg(args, u64);
			NumberToHexString(number, hexOutput);

			if (logger->FramebufferEnabled)
				FramebufferWriteString(&logger->Framebuffer, hexOutput);

			if (logger->SerialConsoleEnabled)
				SerialConsoleWriteString(&logger->SerialConsole, hexOutput);

			i++;
		} else {
			if (logger->FramebufferEnabled)
				FramebufferWriteChar(&logger->Framebuffer, format[i]);

			if (logger->SerialConsoleEnabled)
				SerialConsoleWriteChar(&logger->SerialConsole, format[i]);
		}

		i++;
	}

	va_end(args);

	if (logger->FramebufferEnabled)
		FramebufferWriteChar(&logger->Framebuffer, '\n');

	if (logger->SerialConsoleEnabled)
		SerialConsoleWriteChar(&logger->SerialConsole, '\n');
}
