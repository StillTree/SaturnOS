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

static void LogChar(Logger* logger, i8 character)
{
	if (logger->FramebufferEnabled)
		FramebufferWriteChar(&logger->Framebuffer, character);

	if (logger->SerialConsoleEnabled)
		SerialConsoleWriteChar(&logger->SerialConsole, character);
}

static void LogString(Logger* logger, const i8* string)
{
	if (logger->FramebufferEnabled)
		FramebufferWriteString(&logger->Framebuffer, string);

	if (logger->SerialConsoleEnabled)
		SerialConsoleWriteString(&logger->SerialConsole, string);
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

	while (*format != '\0') {
		if (*format == '%') {
			format++;
			switch (*format) {
			case 'c': {
				char c = (char)va_arg(args, i32);
				LogChar(logger, c);
				break;
			}
			case 's': {
				const i8* string = va_arg(args, i8*);
				LogString(logger, string);
				break;
			}
			// For now `%x` assumes an unsigned integer
			// TODO: support signed
			case 'x': {
				i8 hexOutput[MAX_HEX_LENGTH];
				u64 number = va_arg(args, u64);
				NumberToHexString(number, hexOutput);
				LogString(logger, hexOutput);
				break;
			}
			case 'u': {
				i8 decOutput[MAX_DECIMAL_LENGTH];
				u64 number = va_arg(args, u64);
				NumberToDecimalString(number, decOutput);
				LogString(logger, decOutput);
				break;
			}
			case 'p': {
				i8 hexOutput[MAX_HEX_LENGTH];
				u64 pointer = va_arg(args, u64);
				NumberToHexString(pointer, hexOutput);
				LogString(logger, hexOutput);
				break;
			}
			case '%': {
				LogChar(logger, '%');
				break;
			}
			default: {
				LogChar(logger, '%');
				LogChar(logger, *format);
				break;
			}
			}

		} else {
			LogChar(logger, *format);
		}

		format++;
	}

	va_end(args);

	if (logger->FramebufferEnabled)
		FramebufferWriteChar(&logger->Framebuffer, '\n');

	if (logger->SerialConsoleEnabled)
		SerialConsoleWriteChar(&logger->SerialConsole, '\n');
}
