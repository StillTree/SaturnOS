#pragma once

#include "Core.h"

typedef struct __attribute__((packed)) InterruptFrame {
	u64 InstructionPointer;
	u64 CodeSegment;
	u64 Flags;
	u64 StackPointer;
	u64 SegmentSelector;
} InterruptFrame;

__attribute__((interrupt)) void BreakpointInterruptHandler(InterruptFrame* frame);

__attribute__((interrupt)) void GeneralProtectionFaultInterruptHandler(InterruptFrame* frame, u64);

__attribute__((interrupt)) void DoubleFaultInterruptHandler(InterruptFrame* frame, u64);

__attribute__((interrupt)) void PageFaultInterruptHandler(InterruptFrame* frame, u64 errorCode);

__attribute__((interrupt)) void KeyboardInterruptHandler(InterruptFrame*);

__attribute__((interrupt)) void TimerInterruptHandler(InterruptFrame*);
