#include "Scheduler.h"

#include "Memory/BitmapFrameAllocator.h"
#include "Memory/Page.h"
#include "Memory/PageTable.h"
#include "Panic.h"
#include "Result.h"

Process g_processes[10];

static Result AllocateProcessStack(Process* process)
{
	// For now, since there is only one thread per process, every thread's stack gets allocated on a predefined address
	Page4KiB stackPage = 0x8000001000;
	for (usz i = 0; i < 20; i++) {
		Frame4KiB stackFrame;
		Result result = AllocateFrame(&g_frameAllocator, &stackFrame);
		if (result) {
			return result;
		}

		PageTableEntry* pml4 = PhysicalAddressAsPointer(process->PML4);
		result = Page4KiBMapTo(pml4, stackPage, stackFrame, PageWriteable | PageUserAccessible);
		if (result) {
			return result;
		}

		stackPage += FRAME_4KIB_SIZE_BYTES;
	}

	return ResultOk;
}

static usz GetProcessID()
{
	static usz id = 1;
	return id++;
}

static usz GetThreadID()
{
	static usz id = 1;
	return id++;
}

static usz FirstUsableProcessIndex()
{
	for (usz i = 0; i < 10; i++) {
		if (g_processes[i].ID == USZ_MAX)
			return i;
	}

	return USZ_MAX;
}

Result DeleteProcess(usz id)
{
	for (usz i = 0; i < 10; i++) {
		if (g_processes[i].ID != id)
			continue;

		MemoryFill(g_processes + i, 0, sizeof(Process));
		g_processes[i].ID = USZ_MAX;
		return ResultOk;
	}

	return ResultSerialOutputUnavailabe;
}

Result CreateProcess(Process** process, void (*entryPoint)())
{
	usz index = FirstUsableProcessIndex();
	if (index == USZ_MAX) {
		return ResultSerialOutputUnavailabe;
	}

	Frame4KiB pml4Frame;
	Result result = AllocateFrame(&g_frameAllocator, &pml4Frame);
	if (result) {
		return result;
	}

	g_processes[index].PML4 = pml4Frame;
	g_processes[index].ID = GetProcessID();
	g_processes[index].Threads[0].ID = GetThreadID();
	result = AllocateProcessStack(g_processes + index);
	if (result) {
		DeallocateFrame(&g_frameAllocator, pml4Frame);
		return result;
	}
	g_processes[index].Threads[0].Status = ThreadReady;

	PageTableEntry* kernelPML4 = PhysicalAddressAsPointer(KernelPageTable4Address());
	PageTableEntry* processPML4 = PhysicalAddressAsPointer(pml4Frame);
	processPML4[511] = kernelPML4[511] & ~PageUserAccessible;

	PhysicalAddress entryPointPhysicalAddress;
	result = VirtualAddressToPhysical((VirtualAddress)entryPoint, kernelPML4, &entryPointPhysicalAddress);
	if (result) {
		DeallocateFrame(&g_frameAllocator, pml4Frame);
		return result;
	}
	g_processes[index].Threads[0].EntryPoint = entryPointPhysicalAddress;

	// TODO: Take care of other identity mapped memory regions so the interrupt handler doesn't absolutely shit itself
	result = Page4KiBMapTo(processPML4, 0xFEE00000, 0xFEE00000, PageWriteable);
	if (result) {
		DeallocateFrame(&g_frameAllocator, pml4Frame);
		return result;
	}

	result = Page4KiBMapTo(
		processPML4, 0x4000000000, Frame4KiBContainingAddress(entryPointPhysicalAddress), PageWriteable | PageUserAccessible);
	if (result) {
		DeallocateFrame(&g_frameAllocator, pml4Frame);
		return result;
	}
	MemoryFill(&g_processes[index].Threads[0].Context, 0, sizeof(CPUContext));
	g_processes[index].Threads[0].Context.CR3 = pml4Frame;
	g_processes[index].Threads[0].Context.InterruptFrame.RIP = 0x4000000000 + VirtualAddressPageOffset((VirtualAddress)entryPoint);
	g_processes[index].Threads[0].Context.InterruptFrame.RSP = (0x8000001000 + (20 * FRAME_4KIB_SIZE_BYTES)) & ~1;
	g_processes[index].Threads[0].Context.RBP = 0;
	g_processes[index].Threads[0].Context.InterruptFrame.RFLAGS = 0x202;

	u16 cs;
	__asm__ volatile("mov %%cs, %0" : "=r"(cs));

	u16 ss;
	__asm__ volatile("mov %%ss, %0" : "=r"(ss));

	// TODO: Take care of this shit, for now I just ignore that completely
	g_processes[index].Threads[0].Context.InterruptFrame.CS = cs;
	g_processes[index].Threads[0].Context.InterruptFrame.SS = ss;

	*process = g_processes + index;

	return ResultOk;
}

Thread* g_currentThread = nullptr;
usz g_currentThreadIndex = 0;

void TestProcess1()
{
	while (true) {
		u32* f = (u32*)0xFFFFFFFF8004B000;

		for (usz i = 0; i < 100000; i++) {
			f[i] = 0x0000ff00;
		}
	}
}

void TestProcess2()
{
	while (true) {
		u32* f = (u32*)0xFFFFFFFF8004B000;

		for (usz i = 0; i < 100000; i++) {
			f[i] = 0x000000ff;
		}
	}
}

Result InitScheduler()
{
	for (usz i = 0; i < 10; i++) {
		g_processes[i].ID = USZ_MAX;
	}

	g_processes[0].ID = 0;
	g_processes[0].Threads[0].ID = 0;
	g_processes[0].Threads[0].Status = ThreadRunning;
	g_processes[0].PML4 = KernelPageTable4Address();
	g_processes[0].Threads[0].Context.CR3 = g_processes[0].PML4;

	g_currentThread = &g_processes[0].Threads[0];
	g_currentThreadIndex = 0;

	Process* process = nullptr;
	Result result = CreateProcess(&process, TestProcess1);
	if (result) {
		return result;
	}

	result = CreateProcess(&process, TestProcess2);
	if (result) {
		return result;
	}

	return ResultOk;
}

void Schedule(CPUContext* cpuContext)
{
	usz startIndex = g_currentThreadIndex;

	do {
		g_currentThreadIndex = (g_currentThreadIndex + 1) % 10;

		if (g_processes[g_currentThreadIndex].ID == USZ_MAX) {
			continue;
		}

		if (g_processes[g_currentThreadIndex].Threads[0].Status != ThreadReady) {
			continue;
		}

		if (g_processes[g_currentThreadIndex].Threads[0].ID == g_currentThread->ID) {
			return;
		}

		Thread* oldThread = g_currentThread;

		g_currentThread = &g_processes[g_currentThreadIndex].Threads[0];

		if (oldThread) {
			oldThread->Status = ThreadReady;
			oldThread->Context = *cpuContext;
		}

		g_currentThread->Status = ThreadRunning;
		*cpuContext = g_currentThread->Context;

		return;
	} while (g_currentThreadIndex != startIndex);

	Hang();
}
