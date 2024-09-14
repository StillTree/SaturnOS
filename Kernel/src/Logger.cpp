#include "Logger.hpp"

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

	void Logger::Log(LogLevel logLevel, const I8* string)
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

			framebuffer.WriteString(string);
			framebuffer.WriteChar('\n');
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

			serialConsole.WriteString(string);
			serialConsole.WriteChar('\n');
		}
	}
}

