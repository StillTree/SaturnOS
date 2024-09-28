#include "Panic.hpp"

#include "Logger.hpp"

namespace SaturnKernel
{
	void Hang()
	{
		while(true)
			__asm__ volatile("cli; hlt");
	}

	void Panic(const I8* message)
	{
		// TODO: Print out the file and line in which the panic was called
		g_mainLogger.framebuffer.Clear();
		g_mainLogger.framebuffer.WriteString("!!!UNRECOVERABLE KERNEL PANIC!!!\n");
		g_mainLogger.framebuffer.WriteString(message);
		g_mainLogger.framebuffer.WriteString("\n\n");
		g_mainLogger.framebuffer.WriteString("Please restart your computer to continue.\n");
		g_mainLogger.framebuffer.WriteString("Hanging...");

		Hang();
	}
}
