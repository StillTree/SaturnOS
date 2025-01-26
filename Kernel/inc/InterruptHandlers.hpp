#pragma once

#include "Core.hpp"

namespace SaturnKernel {

struct __attribute__((packed)) InterruptFrame {
	u64 InstructionPointer;
	u64 CodeSegment;
	u64 Flags;
	u64 StackPointer;
	u64 SegmentSelector;
};

__attribute__((interrupt)) auto BreakpointInterruptHandler(InterruptFrame* frame) -> void;
[[noreturn]]
__attribute__((interrupt)) auto DoubleFaultInterruptHandler(InterruptFrame* frame, u64) -> void;
[[noreturn]]
__attribute__((interrupt)) auto PageFaultInterruptHandler(InterruptFrame* frame, u64 errorCode) -> void;

__attribute__((interrupt)) auto KeyboardInterruptHandler(InterruptFrame*) -> void;

__attribute__((interrupt)) void TestInterruptHandler(InterruptFrame*);

}
