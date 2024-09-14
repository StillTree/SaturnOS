#pragma once

#include "Core.hpp"
#include "Logger/Framebuffer.hpp"
#include "Logger/SerialConsole.hpp"

namespace SaturnKernel
{
	enum class LogLevel
	{
		Debug,
		Info,
		Warn,
		Error
	};

	struct Logger
	{
		FramebufferLogger   framebuffer;
		SerialConsoleLogger serialConsole;

		bool framebufferEnabled;
		bool serialConsoleEnabled;

		void Init(
			bool framebufferEnabled,
			bool serialConsoleEnabled,
			KernelBootInfo* bootInfo,
			U16 serialConsolePort);
		void Log(LogLevel logLevel, const I8* string);
	};

	extern Logger g_mainLogger;
}

#define SK_LOG_DEBUG(str) SaturnKernel::g_mainLogger.Log(SaturnKernel::LogLevel::Debug, str)
#define SK_LOG_INFO(str)  SaturnKernel::g_mainLogger.Log(SaturnKernel::LogLevel::Info, str)
#define SK_LOG_WARN(str)  SaturnKernel::g_mainLogger.Log(SaturnKernel::LogLevel::Warn, str)
#define SK_LOG_ERROR(str) SaturnKernel::g_mainLogger.Log(SaturnKernel::LogLevel::Error, str)

