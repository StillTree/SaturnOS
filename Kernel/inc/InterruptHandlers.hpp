#pragma once

#include "Core.hpp"

namespace SaturnKernel {

struct __attribute__((packed)) InterruptFrame {
	U64 InstructionPointer;
	U64 CodeSegment;
	U64 Flags;
	U64 StackPointer;
	U64 SegmentSelector;
};

__attribute__((interrupt)) auto BreakpointInterruptHandler(InterruptFrame* frame) -> void;
[[noreturn]]
__attribute__((interrupt)) auto DoubleFaultInterruptHandler(InterruptFrame* frame, U64) -> void;
[[noreturn]]
__attribute__((interrupt)) auto PageFaultInterruptHandler(InterruptFrame* frame, U64 errorCode) -> void;

__attribute__((interrupt)) auto KeyboardInterruptHandler(InterruptFrame*) -> void;

}
