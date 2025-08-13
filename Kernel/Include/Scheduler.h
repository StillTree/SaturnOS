#pragma once

#include "Core.h"
#include "InterruptHandlers.h"
#include "Memory/Frame.h"
#include "Memory/SizedBlockAllocator.h"
#include "Memory/VirtualMemoryAllocator.h"

constexpr usz MAX_THREADS_PER_PROCESS = 64;
constexpr usz MAX_PROCESSES = 64;
constexpr usz MAX_FILE_DESCRIPTORS = 64;
// 100 KiB
constexpr usz THREAD_USER_STACK_SIZE_BYTES = 102400;
constexpr usz THREAD_KERNEL_STACK_SIZE_BYTES = 20480;

// I have no clue if this is enough, but for now it should suffice I guess...
typedef struct CPUContext {
	u64 CR3;
	u64 RAX;
	u64 RBX;
	u64 RCX;
	u64 RDX;
	u64 RSI;
	u64 RDI;
	u64 RBP;
	u64 R8;
	u64 R9;
	u64 R10;
	u64 R11;
	u64 R12;
	u64 R13;
	u64 R14;
	u64 R15;
	// TODO: TLS
	// u64 FS;
	// u64 GS;
	InterruptFrame InterruptFrame;
} CPUContext;

typedef enum ThreadStatus : u8 {
	ThreadStartingUp = 1,
	ThreadReady,
	ThreadRunning,
	ThreadDead,
	// TODO: Implement thread sleeping
	ThreadSleeping
} ThreadStatus;

typedef struct Thread {
	usz ID;
	ThreadStatus Status;
	CPUContext Context;
	/// A 100 KiB stack for use in the userspace.
	Page4KiB UserStackTop;
	/// A 20 KiB stack for use in the syscalls, interrupts or anything else that the kernel must do.
	Page4KiB KernelStackTop;
	struct Process* ParentProcess;
} Thread;

typedef struct Process {
	usz ID;
	Frame4KiB PML4;
	/// Maximum of 64 threads per process.
	Thread* Threads[64];
	Thread* MainThread;
	usz ThreadCount;
	VirtualMemoryAllocator VirtualMemoryAllocator;
	/// A list of files opened by the process.
	SizedBlockAllocator FileDescriptors;
	SizedBlockAllocator ELFSegmentMap;
} Process;

typedef struct Scheduler {
	SizedBlockAllocator Processes;
	SizedBlockAllocator Threads;
	Thread* CurrentThread;
} Scheduler;

Result InitScheduler();

/// This functions should be called only when the interrupt flag is cleared. It can be set afterwards.
Result ProcessCreate(Process** createdProcess);
/// This functions should be called only when the interrupt flag is cleared. It can be set afterwards.
Result ProcessTerminateStart(Process* process);
/// This functions should be called only when the interrupt flag is cleared. It can be set afterwards.
Result ProcessTerminateFinish(Process* process);
/// This functions should be called only when the interrupt flag is cleared. It can be set afterwards.
void ThreadLaunch(Thread* thread);
/// This functions should be called only when the interrupt flag is cleared. It can be set afterwards.
Result ThreadTerminateStart(Thread* thread);
/// This functions should be called only when the interrupt flag is cleared. It can be set afterwards.
Result ThreadTerminateFinish(Thread* thread);

void ScheduleInterrupt(CPUContext* cpuContext);
void ScheduleException(CPUContext* cpuContext);
/// Does everything except for loading the interrupt frame.
void ScheduleSyscallStart();
/// Loads the interrupt frame.
void ScheduleSyscallFinish(CPUContext* cpuContext);

extern Scheduler g_scheduler;
