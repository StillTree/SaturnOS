#include "Logger.hpp"
#include "Format.hpp"
#include <stdarg.h>

namespace SaturnKernel
{
	Logger g_mainLogger;

	void Logger::Init(bool framebufferEnabled, bool serialConsoleEnabled, KernelBootInfo& bootInfo, U16 serialConsolePort)
	{
		this->FramebufferEnabled   = framebufferEnabled;
		this->SerialConsoleEnabled = serialConsoleEnabled;

		if(framebufferEnabled)
		{
			Framebuffer = { .Framebuffer	 = reinterpret_cast<U32*>(bootInfo.FramebufferAddress),
							.FramebufferSize = bootInfo.FramebufferSize,
							.Width			 = bootInfo.FramebufferWidth,
							.Height			 = bootInfo.FramebufferHeight,
							.CursorPositionX = 0,
							.CursorPositionY = 0 };
		}

		if(serialConsoleEnabled)
		{
			SerialConsole.Init(serialConsolePort);
		}

		Framebuffer.Clear();
	}

	void Logger::Log(LogLevel logLevel, const I8* format, ...)
	{
		if(FramebufferEnabled)
		{
			switch(logLevel)
			{
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

		if(SerialConsoleEnabled)
		{
			switch(logLevel)
			{
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

		USIZE i = 0;
		while(format[i] != '\0')
		{
			if(format[i] == '{' && format[i + 1] == '}')
			{
				I8 hexOutput[MAX_HEX_LENGTH];
				U64 number = va_arg(args, U64);
				NumberToHexString(number, hexOutput);

				if(FramebufferEnabled)
					Framebuffer.WriteString(hexOutput);

				if(SerialConsoleEnabled)
					SerialConsole.WriteString(hexOutput);

				i++;
			}
			else
			{
				if(FramebufferEnabled)
					Framebuffer.WriteChar(format[i]);

				if(SerialConsoleEnabled)
					SerialConsole.WriteChar(format[i]);
			}

			i++;
		}

		va_end(args);

		if(FramebufferEnabled)
			Framebuffer.WriteChar('\n');

		if(SerialConsoleEnabled)
			SerialConsole.WriteChar('\n');
	}
}
