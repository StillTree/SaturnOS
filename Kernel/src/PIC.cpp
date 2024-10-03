#include "PIC.hpp"

#include "InOut.hpp"

namespace SaturnKernel
{
	void EOISignal(U8 interruptVector)
	{
		if(interruptVector >= 8)
			OutputU8(PIC_SLAVE_COMMAND, PIC_EOI_SIGNAL);

		OutputU8(PIC_MASTER_COMMAND, PIC_EOI_SIGNAL);
	}

	void SetIRQMask(IRQMask mask)
	{
		OutputU8(PIC_MASTER_DATA, ~static_cast<U16>(mask));
		OutputU8(PIC_SLAVE_DATA, ~(static_cast<U16>(mask) >> 8));
	}

	void ReinitializePIC()
	{
		// Start the initialization process
		OutputU8(PIC_MASTER_COMMAND, 0x11);
		IOWait();
		OutputU8(PIC_SLAVE_COMMAND, 0x11);
		IOWait();

		// Tell them their interrupt vector offsets
		OutputU8(PIC_MASTER_DATA, PIC_OFFSET);
		IOWait();
		OutputU8(PIC_SLAVE_DATA, PIC_OFFSET + 8);
		IOWait();

		// Setup them in cascaded mode and tell them how they're wired up
		OutputU8(PIC_MASTER_DATA, 4);
		IOWait();
		OutputU8(PIC_SLAVE_DATA, 2);
		IOWait();

		// Set them in 8086 mode
		OutputU8(PIC_MASTER_DATA, 1);
		IOWait();
		OutputU8(PIC_SLAVE_DATA, 1);
		IOWait();

		SetIRQMask(IRQMask::Keyboard);
	}
}
