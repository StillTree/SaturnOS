#pragma once

#include "Core.hpp"

namespace SaturnKernel
{
	struct __attribute__((packed)) InterruptFrame
	{
		U64 InstructionPointer;
		U64 CodeSegment;
		U64 Flags;
		U64 StackPointer;
		U64 SegmentSelector;
	};

	__attribute__((interrupt)) void BreakpointInterruptHandler(InterruptFrame* frame);
	[[noreturn]]
	__attribute__((interrupt)) void DoubleFaultInterruptHandler(InterruptFrame* frame, U64);
	[[noreturn]]
	__attribute__((interrupt)) void PageFaultInterruptHandler(InterruptFrame* frame, U64 errorCode);
}
