#include "Scheduler.h"

#include "ELFLoader.h"
#include "GDT.h"
#include "Memory.h"
#include "Memory/BitmapFrameAllocator.h"
#include "Memory/Page.h"
#include "Memory/PageTable.h"
#include "Memory/SizedBlockAllocator.h"
#include "Result.h"
#include "Storage/VirtualFileSystem.h"

Scheduler g_scheduler;

static Result AllocateThreadStack(Process* process, usz size, PageTableEntryFlags flags, Page4KiB* stackTop)
{
	// TODO: Add random stack offset support (subtract a random number between 0 and 4096 from the stack top and align it to 16 bytes)
	void* stackBottom;
	Result result = AllocateBackedVirtualMemory(&process->VirtualMemoryAllocator, size, flags, &stackBottom);
	if (result) {
		return result;
	}

	*stackTop = (Page4KiB)stackBottom + size;

	return result;
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

Result ProcessTerminateStart(Process* process)
{
	Result result = ResultOk;

	for (usz i = 0; i < MAX_THREADS_PER_PROCESS; i++) {
		if (!process->Threads[i]) {
			continue;
		}

		result = ThreadTerminateStart(process->Threads[i]);
		if (result) {
			return result;
		}
	}

	ELFSegmentRegion* elfSegmentRegionIter = nullptr;
	while (!SizedBlockIterate(&process->ELFSegmentMap, (void**)&elfSegmentRegionIter)) {
		usz size = elfSegmentRegionIter->End - elfSegmentRegionIter->Begin;
		result = DeallocateBackedVirtualMemory(&process->VirtualMemoryAllocator, (void*)elfSegmentRegionIter->Begin, size);
		if (result) {
			return result;
		}
	}

	// As long as all of the virtual memory allocator's regions have been properly freed, its backing memory can simply be deallocated
	// The bitmaps always start at the exact beginning of the backing memory pool, so I can just point to them when deallocating
	result = DeallocateBackedVirtualMemory(&g_kernelMemoryAllocator, process->ELFSegmentMap.BlockBitmap, PAGE_4KIB_SIZE_BYTES);
	if (result) {
		return result;
	}

	ProcessFileDescriptor* fileDescriptorIter = nullptr;
	while (!SizedBlockIterate(&process->FileDescriptors, (void**)&fileDescriptorIter)) {
		usz index = SizedBlockGetIndex(&process->FileDescriptors, fileDescriptorIter);

		result = FileClose(&g_virtualFileSystem, index);
		if (result) {
			return result;
		}
	}
	result = DeallocateBackedVirtualMemory(&g_kernelMemoryAllocator, process->FileDescriptors.BlockBitmap, PAGE_4KIB_SIZE_BYTES);
	if (result) {
		return result;
	}

	return result;
}

Result ProcessTerminateFinish(Process* process)
{
	Result result = ResultOk;

	for (usz i = 0; i < MAX_THREADS_PER_PROCESS; i++) {
		if (!process->Threads[i]) {
			continue;
		}

		result = ThreadTerminateFinish(process->Threads[i]);
		if (result) {
			return result;
		}
	}

	result
		= DeallocateBackedVirtualMemory(&g_kernelMemoryAllocator, process->VirtualMemoryAllocator.ListBackingStorage.BlockBitmap, 102400);
	if (result) {
		return result;
	}

	// Deallocate the process's PML4
	// TODO: Deallocate the intermediate frames in the page table setup (levels 1, 2 and 3)
	DeallocateFrame(&g_frameAllocator, process->PML4);

	result = SizedBlockDeallocate(&g_scheduler.Processes, process);
	if (result) {
		return result;
	}

	return result;
}

Result ThreadTerminateStart(Thread* thread)
{
	Process* process = thread->ParentProcess;
	thread->Status = ThreadDead;

	Result result = DeallocateBackedVirtualMemory(
		&process->VirtualMemoryAllocator, (u8*)thread->UserStackTop - THREAD_USER_STACK_SIZE_BYTES, THREAD_USER_STACK_SIZE_BYTES);
	if (result) {
		return result;
	}

	return result;
}

Result ThreadTerminateFinish(Thread* thread)
{
	if (thread->Status != ThreadDead) {
		return ResultSerialOutputUnavailable;
	}

	Process* process = thread->ParentProcess;

	Result result = DeallocateBackedVirtualMemory(
		&process->VirtualMemoryAllocator, (u8*)thread->KernelStackTop - THREAD_KERNEL_STACK_SIZE_BYTES, THREAD_KERNEL_STACK_SIZE_BYTES);
	if (result) {
		return result;
	}

	result = SizedBlockDeallocate(&g_scheduler.Threads, thread);
	if (result) {
		return result;
	}

	process->ThreadCount--;

	return result;
}

Result ProcessCreate(Process** createdProcess)
{
	Process* process = nullptr;
	Result result = SizedBlockAllocate(&g_scheduler.Processes, (void**)&process);
	if (result) {
		return result;
	}

	void* fileDescriptorsPool;
	result = AllocateBackedVirtualMemory(&g_kernelMemoryAllocator, PAGE_4KIB_SIZE_BYTES, PageWriteable, &fileDescriptorsPool);
	if (result) {
		return result;
	}

	result = InitSizedBlockAllocator(&process->FileDescriptors, fileDescriptorsPool, PAGE_4KIB_SIZE_BYTES, sizeof(ProcessFileDescriptor));
	if (result) {
		return result;
	}

	void* elfSegmentMapPool;
	result = AllocateBackedVirtualMemory(&g_kernelMemoryAllocator, PAGE_4KIB_SIZE_BYTES, PageWriteable, &elfSegmentMapPool);
	if (result) {
		return result;
	}

	result = InitSizedBlockAllocator(&process->ELFSegmentMap, elfSegmentMapPool, PAGE_4KIB_SIZE_BYTES, sizeof(ELFSegmentRegion));
	if (result) {
		return result;
	}

	Frame4KiB pml4Frame = AllocateFrame(&g_frameAllocator);
	PageTableEntry* processPML4 = PhysicalAddressAsPointer(pml4Frame);
	InitEmptyPageTable(processPML4);

	void* virtualMemoryAllocatorPool;
	result = AllocateBackedVirtualMemory(&g_kernelMemoryAllocator, 102400, PageWriteable, &virtualMemoryAllocatorPool);
	if (result) {
		return result;
	}

	result = InitVirtualMemoryAllocator(&process->VirtualMemoryAllocator, virtualMemoryAllocatorPool, 102400, pml4Frame);
	if (result) {
		return result;
	}

	result = MarkVirtualMemoryUsed(&process->VirtualMemoryAllocator, 0x800000000000, U64_MAX - PAGE_4KIB_SIZE_BYTES + 1);
	if (result) {
		return result;
	}

	Thread* mainThread = nullptr;
	result = SizedBlockAllocate(&g_scheduler.Threads, (void**)&mainThread);
	if (result) {
		return result;
	}

	process->PML4 = pml4Frame;
	process->ID = GetProcessID();
	for (usz i = 0; i < MAX_THREADS_PER_PROCESS; i++) {
		process->Threads[i] = nullptr;
	}
	process->Threads[0] = mainThread;
	process->MainThread = mainThread;

	PageTableEntry* kernelPML4 = PhysicalAddressAsPointer(g_bootInfo.KernelPML4);
	processPML4[510] = kernelPML4[510] & ~PageUserAccessible;
	processPML4[511] = kernelPML4[511] & ~PageUserAccessible;

	mainThread->ID = GetThreadID();
	mainThread->ParentProcess = process;

	Page4KiB userStackTop;
	result = AllocateThreadStack(process, THREAD_USER_STACK_SIZE_BYTES, PageWriteable | PageUserAccessible, &userStackTop);
	if (result) {
		return result;
	}
	mainThread->UserStackTop = userStackTop;

	Page4KiB kernelStackTop;
	result = AllocateThreadStack(process, THREAD_KERNEL_STACK_SIZE_BYTES, PageWriteable, &kernelStackTop);
	if (result) {
		return result;
	}
	mainThread->KernelStackTop = kernelStackTop;

	MemoryFill(&mainThread->Context, 0, sizeof(CPUContext));
	mainThread->Context.CR3 = pml4Frame;
	mainThread->Context.InterruptFrame.RSP = userStackTop;
	mainThread->Context.RBP = 0;
	mainThread->Context.InterruptFrame.RFLAGS = 0x202;

	mainThread->Context.InterruptFrame.CS = GDT_ENTRY_USER_CODE;
	mainThread->Context.InterruptFrame.SS = GDT_ENTRY_USER_DATA;
	mainThread->Status = ThreadStartingUp;

	*createdProcess = process;

	return result;
}

void ThreadLaunch(Thread* thread) { thread->Status = ThreadReady; }

Result InitScheduler()
{
	usz processPoolSize = sizeof(Process) * MAX_PROCESSES;
	void* processPool;
	Result result = AllocateBackedVirtualMemory(&g_kernelMemoryAllocator, Page4KiBNext(processPoolSize), PageWriteable, &processPool);
	if (result) {
		return result;
	}

	result = InitSizedBlockAllocator(&g_scheduler.Processes, processPool, processPoolSize, sizeof(Process));
	if (result) {
		return result;
	}

	usz threadPoolSize = sizeof(Thread) * MAX_PROCESSES * MAX_THREADS_PER_PROCESS;
	void* threadPool;
	result = AllocateBackedVirtualMemory(&g_kernelMemoryAllocator, Page4KiBNext(threadPoolSize), PageWriteable, &threadPool);
	if (result) {
		return result;
	}

	result = InitSizedBlockAllocator(&g_scheduler.Threads, threadPool, threadPoolSize, sizeof(Thread));
	if (result) {
		return result;
	}

	Process* kernelProcess = nullptr;
	result = SizedBlockAllocate(&g_scheduler.Processes, (void**)&kernelProcess);
	if (result) {
		return result;
	}

	kernelProcess->ID = 0;
	kernelProcess->PML4 = g_bootInfo.KernelPML4;
	kernelProcess->ThreadCount = 1;
	for (usz i = 0; i < MAX_THREADS_PER_PROCESS; i++) {
		kernelProcess->Threads[i] = nullptr;
	}

	Thread* kernelMainThread = nullptr;
	result = SizedBlockAllocate(&g_scheduler.Threads, (void**)&kernelMainThread);
	if (result) {
		return result;
	}

	kernelProcess->Threads[0] = kernelMainThread;
	kernelMainThread->ID = 0;
	kernelMainThread->Status = ThreadRunning;
	kernelMainThread->Context.CR3 = kernelProcess->PML4;
	g_scheduler.CurrentThread = kernelMainThread;
	kernelMainThread->UserStackTop = g_bootInfo.KernelStackTop;
	kernelMainThread->KernelStackTop = (Page4KiB)g_kernelInterruptStack + sizeof(g_kernelInterruptStack);
	kernelMainThread->ParentProcess = kernelProcess;

	void* fileDescriptorsPool;
	result = AllocateBackedVirtualMemory(&g_kernelMemoryAllocator, PAGE_4KIB_SIZE_BYTES, PageWriteable, &fileDescriptorsPool);
	if (result) {
		return result;
	}

	result = InitSizedBlockAllocator(
		&kernelProcess->FileDescriptors, fileDescriptorsPool, PAGE_4KIB_SIZE_BYTES, sizeof(ProcessFileDescriptor));
	if (result) {
		return result;
	}

	return result;
}

void ProcessStepInto(Process* process)
{
	__asm__ volatile("movq %0, %%cr3" :: "r"(process->PML4) : "memory");
}

void ProcessStepOut()
{
	__asm__ volatile("movq %0, %%cr3" :: "r"(g_bootInfo.KernelPML4) : "memory");
}

void ScheduleInterrupt(CPUContext* cpuContext)
{
	Thread* threadIterator = g_scheduler.CurrentThread;
	// TODO: Temporary workaround for when only one process is being run
	if (g_scheduler.CurrentThread->ParentProcess->ID == 0) {
		g_scheduler.CurrentThread->Status = ThreadReady;
	}

	while (!SizedBlockCircularIterate(&g_scheduler.Threads, (void**)&threadIterator)) {
		if (threadIterator->Status != ThreadReady) {
			continue;
		}

		Thread* oldThread = g_scheduler.CurrentThread;

		g_scheduler.CurrentThread = threadIterator;

		oldThread->Status = ThreadReady;
		oldThread->Context = *cpuContext;

		g_scheduler.CurrentThread->Status = ThreadRunning;
		*cpuContext = g_scheduler.CurrentThread->Context;

		g_tss.RSP[0] = g_scheduler.CurrentThread->KernelStackTop;

		return;
	}
}

void ScheduleDiscardStart()
{
	Thread* threadIterator = g_scheduler.CurrentThread;

	while (!SizedBlockCircularIterate(&g_scheduler.Threads, (void**)&threadIterator)) {
		if (threadIterator->Status != ThreadReady) {
			continue;
		}

		g_scheduler.CurrentThread = threadIterator;

		g_scheduler.CurrentThread->Status = ThreadRunning;

		g_tss.RSP[0] = g_scheduler.CurrentThread->KernelStackTop;

		return;
	}
}

void ScheduleDiscardFinish(CPUContext* cpuContext) { *cpuContext = g_scheduler.CurrentThread->Context; }
