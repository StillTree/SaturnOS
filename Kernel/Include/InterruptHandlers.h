#pragma once

#include "Core.h"

typedef struct __attribute__((packed)) InterruptFrame {
	u64 RIP;
	u64 CS;
	u64 RFLAGS;
	u64 RSP;
	u64 SS;
} InterruptFrame;

__attribute__((interrupt)) void BreakpointInterruptHandler(InterruptFrame* frame);

__attribute__((interrupt)) void InvalidOpcodeInterruptHandler(InterruptFrame* frame);

__attribute__((interrupt)) void GeneralProtectionFaultInterruptHandler(InterruptFrame* frame, u64);

__attribute__((interrupt)) void DoubleFaultInterruptHandler(InterruptFrame* frame, u64);

__attribute__((interrupt)) void PageFaultInterruptHandler(InterruptFrame* frame, u64 errorCode);

__attribute__((interrupt)) void KeyboardInterruptHandler(InterruptFrame*);

void ScheduleInterruptHandler();
void ScheduleExceptionHandler();
