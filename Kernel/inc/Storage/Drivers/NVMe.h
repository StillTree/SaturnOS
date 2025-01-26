#include "Core.h"
#include "Result.h"

namespace SaturnKernel {

struct NVMeRegisters {
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
};

struct NVMeSubmissionEntry {
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
};

struct NVMeCompletionEntry {
	u32 DW0;
	u32 DW1;
	u16 SQHD;
	u16 SQID;
	u16 CID;
	u16 STATUS;
};

struct NVMeDriver {
	auto Init() -> Result<void>;
	auto SendAdminCommand(NVMeSubmissionEntry command) -> Result<void>;

	NVMeRegisters* Registers = nullptr;

	NVMeSubmissionEntry* AdminSubmissionQueue = nullptr;
	NVMeCompletionEntry* AdminCompletionQueue = nullptr;

	u32 DoorbellStride = 4;
	u16 AdminSubmissionHead = 0;
	u16 AdminSubmissionTail = 0;
	u16 AdminCompletionHead = 0;
	bool AdminPhase = true;

	static constexpr u16 QUEUE_SIZE = 64;
};

extern NVMeDriver g_nvmeDriver;

}
