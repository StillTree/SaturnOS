#include "Scheduler.h"

#include "Memory.h"
#include "Memory/BitmapFrameAllocator.h"
#include "Memory/Page.h"
#include "Memory/PageTable.h"
#include "Result.h"
#include "Storage/VirtualFileSystem.h"

Scheduler g_scheduler;

static Result AllocateThreadStack(Process* process, Page4KiB* stackTop)
{
	// TODO: Add random stack offset support (subtract a random number between 0 and 4096 from the stack top and align it to 16 bytes)
	Page4KiB stackBottom;
	Result result = AllocateBackedVirtualMemory(
		&process->VirtualMemoryAllocator, THREAD_STACK_SIZE_BYTES, PageWriteable | PageUserAccessible | PageNoExecute, &stackBottom);
	if (result) {
		return result;
	}

	*stackTop = stackBottom + THREAD_STACK_SIZE_BYTES;

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

Result ProcessRemove(Scheduler* scheduler, Process* process)
{
	Result result = ResultOk;

	for (usz i = 0; i < MAX_THREADS_PER_PROCESS; i++) {
		if (!process->Threads[i]) {
			continue;
		}

		process->Threads[i]->Status = ThreadDead;

		// Deallocate the process's stack
		result = DeallocateBackedVirtualMemory(
			&process->VirtualMemoryAllocator, process->Threads[i]->StackTop - THREAD_STACK_SIZE_BYTES, THREAD_STACK_SIZE_BYTES);
		if (result) {
			return result;
		}

		// Deallocate the entry point
		// result = DeallocateMMIORegion(&process->VirtualMemoryAllocator, process->Threads[i]->EntryPointPage, PAGE_4KIB_SIZE_BYTES);
		// if (result) {
		// 	return result;
		// }

		result = SizedBlockDeallocate(&scheduler->Threads, process->Threads[i]);
		if (result) {
			return result;
		}
	}

	// As long as all of the virtual memory allocator's regions have been properly freed, its backing memory can simply be deallocated.
	result = DeallocateBackedVirtualMemory(&g_kernelMemoryAllocator, process->AllocatorBackingMemory, 102400);
	if (result) {
		return result;
	}

	// Deallocate the process's PML4
	DeallocateFrame(&g_frameAllocator, process->PML4);

	result = SizedBlockDeallocate(&scheduler->Processes, process);
	if (result) {
		return result;
	}

	return result;
}

Result ProcessCreate(Scheduler* scheduler, Process** createdProcess)
{
	Process* process = nullptr;
	Result result = SizedBlockAllocate(&scheduler->Processes, (void**)&process);
	if (result) {
		return result;
	}

	usz fileDescriptorsPoolSize = Page4KiBNext(sizeof(ProcessFileDescriptor) * MAX_FILE_DESCRIPTORS);
	Page4KiB fileDescriptorsPool;
	result = AllocateBackedVirtualMemory(&g_kernelMemoryAllocator, fileDescriptorsPoolSize, PageWriteable, &fileDescriptorsPool);
	if (result) {
		return result;
	}

	result
		= InitSizedBlockAllocator(&process->FileDescriptors, fileDescriptorsPool, fileDescriptorsPoolSize, sizeof(ProcessFileDescriptor));
	if (result) {
		return result;
	}

	Frame4KiB pml4Frame;
	result = AllocateFrame(&g_frameAllocator, &pml4Frame);
	if (result) {
		return result;
	}
	PageTableEntry* processPML4 = PhysicalAddressAsPointer(pml4Frame);
	InitEmptyPageTable(processPML4);

	result = AllocateBackedVirtualMemory(&g_kernelMemoryAllocator, 102400, PageWriteable, &process->AllocatorBackingMemory);
	if (result) {
		return result;
	}

	result = InitVirtualMemoryAllocator(&process->VirtualMemoryAllocator, process->AllocatorBackingMemory, 102400, pml4Frame);
	if (result) {
		return result;
	}

	result = MarkVirtualMemoryUsed(&process->VirtualMemoryAllocator, 0x800000000000, U64_MAX - PAGE_4KIB_SIZE_BYTES + 1);
	if (result) {
		return result;
	}

	Thread* mainThread = nullptr;
	result = SizedBlockAllocate(&scheduler->Threads, (void**)&mainThread);
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

	PageTableEntry* kernelPML4 = PhysicalAddressAsPointer(KernelPML4());
	processPML4[510] = kernelPML4[510] & ~PageUserAccessible;
	processPML4[511] = kernelPML4[511] & ~PageUserAccessible;

	mainThread->ID = GetThreadID();
	mainThread->ParentProcess = process;

	Page4KiB stackTop;
	result = AllocateThreadStack(process, &stackTop);
	if (result) {
		return result;
	}
	mainThread->StackTop = stackTop;

	MemoryFill(&mainThread->Context, 0, sizeof(CPUContext));
	mainThread->Context.CR3 = pml4Frame;
	mainThread->Context.InterruptFrame.RSP = stackTop;
	mainThread->Context.RBP = 0;
	mainThread->Context.InterruptFrame.RFLAGS = 0x202;

	mainThread->Context.InterruptFrame.CS = 0x23;
	mainThread->Context.InterruptFrame.SS = 0x1b;
	mainThread->Status = ThreadStartingUp;

	*createdProcess = process;

	return result;
}

void ThreadLaunch(Thread* thread)
{
	thread->Status = ThreadReady;
}

Result InitScheduler(Scheduler* scheduler)
{
	usz processPoolSize = sizeof(Process) * MAX_PROCESSES;
	Page4KiB processPool;
	Result result = AllocateBackedVirtualMemory(&g_kernelMemoryAllocator, Page4KiBNext(processPoolSize), PageWriteable, &processPool);
	if (result) {
		return result;
	}

	result = InitSizedBlockAllocator(&scheduler->Processes, processPool, processPoolSize, sizeof(Process));
	if (result) {
		return result;
	}

	usz threadPoolSize = sizeof(Thread) * MAX_PROCESSES * MAX_THREADS_PER_PROCESS;
	Page4KiB threadPool;
	result = AllocateBackedVirtualMemory(&g_kernelMemoryAllocator, Page4KiBNext(threadPoolSize), PageWriteable, &threadPool);
	if (result) {
		return result;
	}

	result = InitSizedBlockAllocator(&scheduler->Threads, threadPool, threadPoolSize, sizeof(Thread));
	if (result) {
		return result;
	}

	Process* kernelProcess = nullptr;
	result = SizedBlockAllocate(&scheduler->Processes, (void**)&kernelProcess);
	if (result) {
		return result;
	}

	kernelProcess->ID = 0;
	kernelProcess->PML4 = KernelPML4();
	for (usz i = 0; i < MAX_THREADS_PER_PROCESS; i++) {
		kernelProcess->Threads[i] = nullptr;
	}

	Thread* kernelMainThread = nullptr;
	result = SizedBlockAllocate(&scheduler->Threads, (void**)&kernelMainThread);
	if (result) {
		return result;
	}

	kernelProcess->Threads[0] = kernelMainThread;
	kernelMainThread->ID = 0;
	kernelMainThread->Status = ThreadRunning;
	kernelMainThread->Context.CR3 = kernelProcess->PML4;
	scheduler->CurrentThread = kernelMainThread;
	kernelMainThread->StackTop = g_bootInfo.KernelStackTop;
	kernelMainThread->ParentProcess = kernelProcess;

	usz fileDescriptorsPoolSize = Page4KiBNext(sizeof(ProcessFileDescriptor) * MAX_FILE_DESCRIPTORS);
	Page4KiB fileDescriptorsPool;
	result = AllocateBackedVirtualMemory(&g_kernelMemoryAllocator, fileDescriptorsPoolSize, PageWriteable, &fileDescriptorsPool);
	if (result) {
		return result;
	}

	result = InitSizedBlockAllocator(
		&kernelProcess->FileDescriptors, fileDescriptorsPool, fileDescriptorsPoolSize, sizeof(ProcessFileDescriptor));
	if (result) {
		return result;
	}

	return result;
}

void ScheduleInterrupt(CPUContext* cpuContext)
{
	// This means only the kernel is running so we can just safely return here
	// if (g_scheduler.Threads.AllocationCount == 1) {
	// 	return;
	// }

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

		return;
	}
}

void ScheduleException(CPUContext* cpuContext)
{
	// This means only the kernel is running so we can just safely return here
	// if (g_scheduler.Threads.AllocationCount == 1) {
	// 	return;
	// }

	Thread* threadIterator = g_scheduler.CurrentThread;

	while (!SizedBlockCircularIterate(&g_scheduler.Threads, (void**)&threadIterator)) {
		if (threadIterator->Status != ThreadReady) {
			continue;
		}

		g_scheduler.CurrentThread = threadIterator;
		g_scheduler.CurrentThread->Status = ThreadRunning;
		*cpuContext = g_scheduler.CurrentThread->Context;

		return;
	}
}
