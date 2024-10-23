#pragma once

#include "Core.hpp"
#include "Logger/Framebuffer.hpp"
#include "Logger/SerialConsole.hpp"

namespace SaturnKernel
{
	enum class LogLevel : U8
	{
		Debug,
		Info,
		Warn,
		Error
	};

	struct Logger
	{
		FramebufferLogger Framebuffer;
		SerialConsoleLogger SerialConsole;

		bool FramebufferEnabled;
		bool SerialConsoleEnabled;

		auto Init(bool framebufferEnabled, bool serialConsoleEnabled, KernelBootInfo& bootInfo, U16 serialConsolePort) -> void;
		auto Log(LogLevel logLevel, const I8* format, ...) -> void;
	};

	extern Logger g_mainLogger;
}

#define SK_LOG_DEBUG(...) SaturnKernel::g_mainLogger.Log(SaturnKernel::LogLevel::Debug, __VA_ARGS__)
#define SK_LOG_INFO(...)  SaturnKernel::g_mainLogger.Log(SaturnKernel::LogLevel::Info, __VA_ARGS__)
#define SK_LOG_WARN(...)  SaturnKernel::g_mainLogger.Log(SaturnKernel::LogLevel::Warn, __VA_ARGS__)
#define SK_LOG_ERROR(...) SaturnKernel::g_mainLogger.Log(SaturnKernel::LogLevel::Error, __VA_ARGS__)
