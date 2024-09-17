#include "Logger.hpp"
#include "Format.hpp"
#include <stdarg.h>

namespace SaturnKernel
{
	Logger g_mainLogger;

	void Logger::Init(
		bool framebufferEnabled,
		bool serialConsoleEnabled,
		KernelBootInfo* bootInfo,
		U16 serialConsolePort)
	{
		this->framebufferEnabled   = framebufferEnabled;
		this->serialConsoleEnabled = serialConsoleEnabled;

		if(framebufferEnabled)
		{
			framebuffer =
				{
					reinterpret_cast<U32*>(bootInfo->framebufferAddress),
					bootInfo->framebufferSize,
					bootInfo->framebufferWidth,
					bootInfo->framebufferHeight,
					0,
					0
				};
		}

		if(serialConsoleEnabled)
		{
			serialConsole.Init(serialConsolePort);
		}

		framebuffer.Clear();
	}

	void Logger::Log(LogLevel logLevel, const I8* format, ...)
	{
		if(framebufferEnabled)
		{
			switch(logLevel)
			{
				case LogLevel::Debug:
					framebuffer.WriteString("[DEBUG]: ");
					break;
				case LogLevel::Info:
					framebuffer.WriteString("[INFO ]: ");
					break;
				case LogLevel::Warn:
					framebuffer.WriteString("[WARN ]: ");
					break;
				case LogLevel::Error:
					framebuffer.WriteString("[ERROR]: ");
					break;
			}
		}

		if(serialConsoleEnabled)
		{
			switch(logLevel)
			{
				case LogLevel::Debug:
					serialConsole.WriteString("[DEBUG]: ");
					break;
				case LogLevel::Info:
					serialConsole.WriteString("[INFO ]: ");
					break;
				case LogLevel::Warn:
					serialConsole.WriteString("[WARN ]: ");
					break;
				case LogLevel::Error:
					serialConsole.WriteString("[ERROR]: ");
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

				if(framebufferEnabled)
					framebuffer.WriteString(hexOutput);

				if(serialConsoleEnabled)
					serialConsole.WriteString(hexOutput);

				i++;
			}
			else
			{
				if(framebufferEnabled)
					framebuffer.WriteChar(format[i]);

				if(serialConsoleEnabled)
					serialConsole.WriteChar(format[i]);
			}

			i++;
		}

		va_end(args);

		if(framebufferEnabled)
			framebuffer.WriteChar('\n');

		if(serialConsoleEnabled)
			serialConsole.WriteChar('\n');
	}
}

