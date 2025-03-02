#include "Core.h"
#include "InterruptHandlers.h"
#include "Memory/Frame.h"
#include "Result.h"

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
	// u64 FS;
	// u64 GS;
	InterruptFrame InterruptFrame;
} CPUContext;

typedef enum ThreadStatus : u8 {
	ThreadReady,
	ThreadRunning,
	ThreadDead,
	// TODO: Implement thread sleeping
	ThreadSleeping
} ThreadStatus;

typedef struct Thread {
	usz ID;
	CPUContext Context;
	ThreadStatus Status;
	/// The first frame of this thread's stack. A process's stack for now is always 20 pages long.
	Frame4KiB Stack;
	PhysicalAddress EntryPoint;
} Thread;

typedef struct Process {
	usz ID;
	Frame4KiB PML4;
	/// For now only 1 thread per process supported.
	Thread Threads[1];
} Process;

Result InitScheduler();
void Schedule(CPUContext* cpuContext);

// "Non-existing" processes get assigned an ID of `USZ_MAX`
extern Process g_processes[10];
extern Thread* g_currentThread;
