#include "Core.h"
#include "Result.h"

typedef struct NVMeRegisters {
	u64 CAP;
	u32 VS;
	u32 INTMS;
	u32 INTMC;
	u32 CC;
	u32 Reserved;
	u32 CSTS;
	u32 NSSR;
	u32 AQA;
	u64 ASQ;
	u64 ACQ;
} NVMeRegisters;

typedef struct NVMeSubmissionEntry {
	u32 CDW0;
	u32 NSID;
	u32 CDW2;
	u32 CDW3;
	u64 MPTR;
	u64 PRP1;
	u64 PRP2;
	u32 CDW10;
	u32 CDW11;
	u32 CDW12;
	u32 CDW13;
	u32 CDW14;
	u32 CDW15;
} NVMeSubmissionEntry;

typedef struct NVMeCompletionEntry {
	u32 DW0;
	u32 DW1;
	u16 SQHD;
	u16 SQID;
	u16 CID;
	u16 STATUS;
} NVMeCompletionEntry;

typedef struct NVMeDriver {
	NVMeRegisters* Registers;

	NVMeSubmissionEntry* AdminSubmissionQueue;
	NVMeCompletionEntry* AdminCompletionQueue;

	u32 DoorbellStride;
	u16 AdminSubmissionTail;
	u16 AdminCompletionHead;
	u16 AdminPhase;

} NVMeDriver;

Result NVMeInit(NVMeDriver* driver);
Result NVMeSendAdminCommand(NVMeDriver* driver, NVMeSubmissionEntry* command);
Result NVMePollNextAdminCompletion(NVMeDriver* driver, NVMeCompletionEntry* entry);

constexpr u16 NVME_QUEUE_SIZE = 64;

extern NVMeDriver g_nvmeDriver;
