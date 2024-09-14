#include "Panic.hpp"

namespace SaturnKernel
{
	void Hang()
	{
		while(true)
			__asm__ volatile("cli; hlt");
	}

	void Panic()
	{
		Hang();
	}
}

