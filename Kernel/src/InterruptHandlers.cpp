#include "InterruptHandlers.hpp"

#include "InOut.hpp"
#include "Logger.hpp"
#include "PIC.hpp"
#include "Panic.hpp"

namespace SaturnKernel
{
	__attribute__((interrupt)) void BreakpointInterruptHandler(InterruptFrame* frame)
	{
		SK_LOG_ERROR("EXCEPTION OCCURED: BREAKPOINT, InterruptFrame");
		SK_LOG_ERROR("{");
		SK_LOG_ERROR("    StackPointer = {}", frame->StackPointer);
		SK_LOG_ERROR("    Flags = {}", frame->Flags);
		SK_LOG_ERROR("    CodeSegment = {}", frame->CodeSegment);
		SK_LOG_ERROR("    SegmentSelector = {}", frame->SegmentSelector);
		SK_LOG_ERROR("    Instruction Pointer = {}", frame->InstructionPointer);
		SK_LOG_ERROR("}");
	}

	__attribute__((interrupt)) void DoubleFaultInterruptHandler(InterruptFrame* frame, U64)
	{
		SK_LOG_ERROR("UNRECOVERABLE EXCEPTION OCCURED: DOUBLE FAULT, InterruptFrame");
		SK_LOG_ERROR("{");
		SK_LOG_ERROR("    StackPointer = {}", frame->StackPointer);
		SK_LOG_ERROR("    Flags = {}", frame->Flags);
		SK_LOG_ERROR("    CodeSegment = {}", frame->CodeSegment);
		SK_LOG_ERROR("    SegmentSelector = {}", frame->SegmentSelector);
		SK_LOG_ERROR("    Instruction Pointer = {}", frame->InstructionPointer);
		SK_LOG_ERROR("}");

		Hang();
	}

	__attribute__((interrupt)) void PageFaultInterruptHandler(InterruptFrame* frame, U64 errorCode)
	{
		U64 faultVirtualAddress;
		__asm__ volatile("mov %%cr2, %0" : "=r"(faultVirtualAddress));

		U64 pml4Address;
		__asm__ volatile("mov %%cr3, %0" : "=r"(pml4Address));

		// TODO: Using the errorCode figure the rest of this shit out

		SK_LOG_ERROR("UNRECOVERABLE EXCEPTION OCCURED: PAGE FAULT");
		SK_LOG_ERROR("Faulty virtual address = {}", faultVirtualAddress);
		SK_LOG_ERROR("PML4 address = {}", pml4Address);
		SK_LOG_ERROR("Error code = {}", errorCode);
		SK_LOG_ERROR("InterruptFrame");
		SK_LOG_ERROR("{");
		SK_LOG_ERROR("    StackPointer = {}", frame->StackPointer);
		SK_LOG_ERROR("    Flags = {}", frame->Flags);
		SK_LOG_ERROR("    CodeSegment = {}", frame->CodeSegment);
		SK_LOG_ERROR("    SegmentSelector = {}", frame->SegmentSelector);
		SK_LOG_ERROR("    Instruction Pointer = {}", frame->InstructionPointer);
		SK_LOG_ERROR("}");

		Hang();
	}

	__attribute__((interrupt)) void KeyboardInterruptHandler(InterruptFrame*)
	{
		U8 scanCode = InputU8(0x60);
		SK_LOG_INFO("{}", scanCode);

		EOISignal(1);
	}
}
