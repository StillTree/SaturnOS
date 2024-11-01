#include "InterruptHandlers.hpp"

#include "InOut.hpp"
#include "Keyboard.hpp"
#include "Logger.hpp"
#include "Memory/PageTable.hpp"
#include "PIC.hpp"
#include "Panic.hpp"

namespace SaturnKernel {

__attribute__((interrupt)) void BreakpointInterruptHandler(InterruptFrame* frame)
{
	SK_LOG_ERROR("EXCEPTION OCCURED: BREAKPOINT, InterruptFrame");
	SK_LOG_ERROR("{");
	SK_LOG_ERROR("\tStackPointer = {}", frame->StackPointer);
	SK_LOG_ERROR("\tFlags = {}", frame->Flags);
	SK_LOG_ERROR("\tCodeSegment = {}", frame->CodeSegment);
	SK_LOG_ERROR("\tSegmentSelector = {}", frame->SegmentSelector);
	SK_LOG_ERROR("\tInstruction Pointer = {}", frame->InstructionPointer);
	SK_LOG_ERROR("}");
}

__attribute__((interrupt)) void DoubleFaultInterruptHandler(InterruptFrame* frame, U64 /*unused*/)
{
	SK_LOG_ERROR("UNRECOVERABLE EXCEPTION OCCURED: DOUBLE FAULT, InterruptFrame");
	SK_LOG_ERROR("{");
	SK_LOG_ERROR("\tStackPointer = {}", frame->StackPointer);
	SK_LOG_ERROR("\tFlags = {}", frame->Flags);
	SK_LOG_ERROR("\tCodeSegment = {}", frame->CodeSegment);
	SK_LOG_ERROR("\tSegmentSelector = {}", frame->SegmentSelector);
	SK_LOG_ERROR("\tInstruction Pointer = {}", frame->InstructionPointer);
	SK_LOG_ERROR("}");

	Hang();
}

__attribute__((interrupt)) void PageFaultInterruptHandler(InterruptFrame* frame, U64 errorCode)
{
	U64 faultVirtualAddress = -1;
	__asm__ volatile("mov %%cr2, %0" : "=r"(faultVirtualAddress));

	U64 pml4Address = PageTable4Address();

	// TODO: Using the errorCode figure the rest of this shit out

	SK_LOG_ERROR("UNRECOVERABLE EXCEPTION OCCURED: PAGE FAULT");
	SK_LOG_ERROR("Faulty virtual address = {}", faultVirtualAddress);
	SK_LOG_ERROR("PML4 address = {}", pml4Address);
	SK_LOG_ERROR("Error code = {}", errorCode);
	SK_LOG_ERROR("InterruptFrame");
	SK_LOG_ERROR("{");
	SK_LOG_ERROR("\tStackPointer = {}", frame->StackPointer);
	SK_LOG_ERROR("\tFlags = {}", frame->Flags);
	SK_LOG_ERROR("\tCodeSegment = {}", frame->CodeSegment);
	SK_LOG_ERROR("\tSegmentSelector = {}", frame->SegmentSelector);
	SK_LOG_ERROR("\tInstruction Pointer = {}", frame->InstructionPointer);
	SK_LOG_ERROR("}");

	Hang();
}

__attribute__((interrupt)) void KeyboardInterruptHandler(InterruptFrame* /*unused*/)
{
	U8 scanCode = InputU8(0x60);
	I8 character = TranslateScanCode(scanCode);
	if (character != '?') {
		g_mainLogger.Framebuffer.WriteChar(character);
		g_mainLogger.SerialConsole.WriteChar(character);
	}

	EOISignal(33);
}

}
