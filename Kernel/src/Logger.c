#include "Logger.h"

#include "Format.h"
#include "Result.h"
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

static void LogResult(Logger* logger, Result result)
{
	if (logger->FramebufferEnabled) {
		switch (result) {
		case ResultOk:
			FramebufferWriteString(&logger->Framebuffer, "ResultOk");
			break;
		case ResultNotEnoughMemoryPages:
			FramebufferWriteString(&logger->Framebuffer, "ResultNotEnoughMemoryPages");
			break;
		case ResultNotEnoughMemoryFrames:
			FramebufferWriteString(&logger->Framebuffer, "ResultNotEnoughMemoryFrames");
			break;
		case ResultSerialOutputUnavailabe:
			FramebufferWriteString(&logger->Framebuffer, "ResultSerialOutputUnavailabe");
			break;
		case ResultOutOfMemory:
			FramebufferWriteString(&logger->Framebuffer, "ResultOutOfMemory");
			break;
		case ResultFrameAlreadyDeallocated:
			FramebufferWriteString(&logger->Framebuffer, "ResultFrameAlreadyDeallocated");
			break;
		case ResultPageAlreadyMapped:
			FramebufferWriteString(&logger->Framebuffer, "ResultPageAlreadyMapped");
			break;
		case ResultPageAlreadyUnmapped:
			FramebufferWriteString(&logger->Framebuffer, "ResultPageAlreadyUnmapped");
			break;
		case ResultHeapBlockTooSmall:
			FramebufferWriteString(&logger->Framebuffer, "ResultHeapBlockTooSmall");
			break;
		case ResultHeapBlockIncorrectAlignment:
			FramebufferWriteString(&logger->Framebuffer, "ResultHeapBlockIncorrectAlignment");
			break;
		case ResultHeapBlockIncorrectSplitSize:
			FramebufferWriteString(&logger->Framebuffer, "ResultHeapBlockIncorrectSplitSize");
			break;
		case ResultInvalidSDTSignature:
			FramebufferWriteString(&logger->Framebuffer, "ResultInvalidSDTSignature");
			break;
		case ResultXSDTCorrupted:
			FramebufferWriteString(&logger->Framebuffer, "ResultXSDTCorrupted");
			break;
		case ResultX2APICUnsupported:
			FramebufferWriteString(&logger->Framebuffer, "ResultX2APICUnsupported");
			break;
		case ResultIOAPICNotPresent:
			FramebufferWriteString(&logger->Framebuffer, "ResultIOAPICNotPresent");
			break;
		case ResultInvalidBARIndex:
			FramebufferWriteString(&logger->Framebuffer, "ResultInvalidBARIndex");
			break;
		case ResultInvalidMSIXVector:
			FramebufferWriteString(&logger->Framebuffer, "ResultInvalidMSIXVector");
			break;
		case ResultPCICapabilitiesUnsupported:
			FramebufferWriteString(&logger->Framebuffer, "ResultPCICapabilitiesNotSupported");
			break;
		case ResultTimeout:
			FramebufferWriteString(&logger->Framebuffer, "ResultTimeout");
			break;
		case ResultOutOfRange:
			FramebufferWriteString(&logger->Framebuffer, "ResultOutOfRange");
			break;
		case ResultAHCI64BitAddressingUnsupported:
			FramebufferWriteString(&logger->Framebuffer, "ResultAHCI64BitAddressingUnsupported");
			break;
		case ResultAHCIDeviceUnsupportedSectorSize:
			FramebufferWriteString(&logger->Framebuffer, "ResultAHCIDeviceUnsupportedSectorSize");
			break;
		default:
			FramebufferWriteString(&logger->Framebuffer, "UnknownResultValue");
			break;
		}
	}

	if (logger->SerialConsoleEnabled) {
		switch (result) {
		case ResultOk:
			SerialConsoleWriteString(&logger->SerialConsole, "ResultOk");
			break;
		case ResultNotEnoughMemoryPages:
			SerialConsoleWriteString(&logger->SerialConsole, "ResultNotEnoughMemoryPages");
			break;
		case ResultNotEnoughMemoryFrames:
			SerialConsoleWriteString(&logger->SerialConsole, "ResultNotEnoughMemoryFrames");
			break;
		case ResultSerialOutputUnavailabe:
			SerialConsoleWriteString(&logger->SerialConsole, "ResultSerialOutputUnavailabe");
			break;
		case ResultOutOfMemory:
			SerialConsoleWriteString(&logger->SerialConsole, "ResultOutOfMemory");
			break;
		case ResultFrameAlreadyDeallocated:
			SerialConsoleWriteString(&logger->SerialConsole, "ResultFrameAlreadyDeallocated");
			break;
		case ResultPageAlreadyMapped:
			SerialConsoleWriteString(&logger->SerialConsole, "ResultPageAlreadyMapped");
			break;
		case ResultPageAlreadyUnmapped:
			SerialConsoleWriteString(&logger->SerialConsole, "ResultPageAlreadyUnmapped");
			break;
		case ResultHeapBlockTooSmall:
			SerialConsoleWriteString(&logger->SerialConsole, "ResultHeapBlockTooSmall");
			break;
		case ResultHeapBlockIncorrectAlignment:
			SerialConsoleWriteString(&logger->SerialConsole, "ResultHeapBlockIncorrectAlignment");
			break;
		case ResultHeapBlockIncorrectSplitSize:
			SerialConsoleWriteString(&logger->SerialConsole, "ResultHeapBlockIncorrectSplitSize");
			break;
		case ResultInvalidSDTSignature:
			SerialConsoleWriteString(&logger->SerialConsole, "ResultInvalidSDTSignature");
			break;
		case ResultXSDTCorrupted:
			SerialConsoleWriteString(&logger->SerialConsole, "ResultXSDTCorrupted");
			break;
		case ResultX2APICUnsupported:
			SerialConsoleWriteString(&logger->SerialConsole, "ResultX2APICUnsupported");
			break;
		case ResultIOAPICNotPresent:
			SerialConsoleWriteString(&logger->SerialConsole, "ResultIOAPICNotPresent");
			break;
		case ResultInvalidBARIndex:
			SerialConsoleWriteString(&logger->SerialConsole, "ResultInvalidBARIndex");
			break;
		case ResultInvalidMSIXVector:
			SerialConsoleWriteString(&logger->SerialConsole, "ResultInvalidMSIXVector");
			break;
		case ResultPCICapabilitiesUnsupported:
			SerialConsoleWriteString(&logger->SerialConsole, "ResultPCICapabilitiesNotSupported");
			break;
		case ResultTimeout:
			SerialConsoleWriteString(&logger->SerialConsole, "ResultTimeout");
			break;
		default:
			SerialConsoleWriteString(&logger->SerialConsole, "ResultUnknownValue");
			break;
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
				i8 decOutput[MAX_DEC_LENGTH];
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
			// My custom format specifier for printing Result values
			case 'r': {
				Result result = (Result)va_arg(args, i32);
				LogResult(logger, result);
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
