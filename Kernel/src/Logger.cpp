#include "Logger.hpp"

#include "Format.hpp"
#include <stdarg.h>

namespace SaturnKernel {

Logger g_mainLogger;

auto Logger::Init(bool framebufferEnabled, bool serialConsoleEnabled, KernelBootInfo& bootInfo, u16 serialConsolePort) -> void
{
	this->FramebufferEnabled = framebufferEnabled;
	this->SerialConsoleEnabled = serialConsoleEnabled;

	if (framebufferEnabled) {
		Framebuffer = { .Framebuffer = bootInfo.Framebuffer,
			.FramebufferSize = bootInfo.FramebufferSize,
			.Width = bootInfo.FramebufferWidth,
			.Height = bootInfo.FramebufferHeight,
			.CursorPositionX = 0,
			.CursorPositionY = 0 };

		Framebuffer.Clear();
	}

	if (serialConsoleEnabled) {
		auto result = SerialConsole.Init(serialConsolePort);
		if (result.IsError()) {
			SK_LOG_WARN("The serial output device at port 0x3f8 is not functioning correctly");
			SerialConsoleEnabled = false;
		}
	}
}

auto Logger::Log(LogLevel logLevel, const i8* format, ...) -> void
{
	if (FramebufferEnabled) {
		switch (logLevel) {
			using enum LogLevel;
		case Debug:
			Framebuffer.WriteString("[DEBUG]: ");
			break;
		case Info:
			Framebuffer.WriteString("[INFO ]: ");
			break;
		case Warn:
			Framebuffer.WriteString("[WARN ]: ");
			break;
		case Error:
			Framebuffer.WriteString("[ERROR]: ");
			break;
		}
	}

	if (SerialConsoleEnabled) {
		switch (logLevel) {
			using enum LogLevel;
		case Debug:
			SerialConsole.WriteString("[DEBUG]: ");
			break;
		case Info:
			SerialConsole.WriteString("[INFO ]: ");
			break;
		case Warn:
			SerialConsole.WriteString("[WARN ]: ");
			break;
		case Error:
			SerialConsole.WriteString("[ERROR]: ");
			break;
		}
	}

	va_list args;
	va_start(args, format);

	usize i = 0;
	while (format[i] != '\0') {
		if (format[i] == '{' && format[i + 1] == '}') {
			i8 hexOutput[MAX_HEX_LENGTH];
			u64 number = va_arg(args, u64);
			NumberToHexString(number, hexOutput);

			if (FramebufferEnabled)
				Framebuffer.WriteString(hexOutput);

			if (SerialConsoleEnabled)
				SerialConsole.WriteString(hexOutput);

			i++;
		} else {
			if (FramebufferEnabled)
				Framebuffer.WriteChar(format[i]);

			if (SerialConsoleEnabled)
				SerialConsole.WriteChar(format[i]);
		}

		i++;
	}

	va_end(args);

	if (FramebufferEnabled)
		Framebuffer.WriteChar('\n');

	if (SerialConsoleEnabled)
		SerialConsole.WriteChar('\n');
}

}
