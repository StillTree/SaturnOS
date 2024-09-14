#include "InterruptHandlers.hpp"

#include "Logger.hpp"
#include "Panic.hpp"

namespace SaturnKernel
{
	__attribute__((interrupt)) void BreakpointInterruptHandler(InterruptFrame* frame)
	{
		SK_LOG_ERROR("EXCEPTION OCCURED: BREAKPOINT, InterruptFrame");
		SK_LOG_ERROR("{");
		SK_LOG_ERROR("}");
	}

	__attribute__((interrupt)) void DoubleFaultInterruptHandler(InterruptFrame* frame, U64)
	{
		SK_LOG_ERROR("UNRECOVERABLE EXCEPTION OCCURED: DOUBLE FAULT, InterruptFrame");
		SK_LOG_ERROR("{");
		SK_LOG_ERROR("}");

		Hang();
	}

	__attribute__((interrupt)) void PageFaultInterruptHandler(InterruptFrame* frame, U64 errorCode)
	{
		U64 faultVirtualAddress;
		__asm__ volatile("mov %%cr2, %0" : "=r"(faultVirtualAddress));

		U64 pml4Address;
		__asm__ volatile("mov %%cr3, %0" : "=r"(pml4Address));

		// TODO: Using the errorCode figure the rest of the shit out

		SK_LOG_ERROR("UNRECOVERABLE EXCEPTION OCCURED: PAGE FAULT, InterruptFrame");
		SK_LOG_ERROR("{");
		SK_LOG_ERROR("}");

		Hang();
	}
}

