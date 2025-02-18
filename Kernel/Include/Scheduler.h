#include "Core.h"
#include "Memory/Frame.h"

// I have no clue if this is enough, but for now it should suffice I guess...
// TODO: Check for other stuff that might need saving like the TSS (or mapping them actually),
// exception handling, do I need to map the kernel? and other shit...
typedef struct CPUContext {
	u64 RSP;
	u64 RIP;
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
	u64 RFLAGS;
	u64 FS;
	u64 GS;
	u64 CS;
	u64 SS;
	u64 CR3;
} CPUContext;

typedef enum ThreadStatus : u8 {
	ThreadInactive,
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
	/// The first frame of this thread's stack.
	Frame4KiB Stack;
} Thread;

typedef struct Process {
	usz ID;
	Frame4KiB PML4;
	/// For now only 1 thread per process supported.
	Thread Threads[1];
} Process;

void InitScheduler();
void Schedule();

// "Non-existing" processes get assigned an ID of 0 for now
Process g_processes[10];
