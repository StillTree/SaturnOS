#include "ELFLoader.h"

#include "Logger.h"
#include "Memory.h"
#include "Memory/VirtualMemoryAllocator.h"
#include "Storage/VirtualFileSystem.h"
#include "elf.h"

Result ProcessLoadELF(Process* process, const i8* elfPath)
{
	usz fileDescriptor;
	Result result = FileOpen(&g_virtualFileSystem, elfPath, OpenFileRead, &fileDescriptor);
	if (result) {
		return result;
	}

	Elf64_Ehdr elfHeader;
	result = FileRead(fileDescriptor, sizeof(elfHeader), &elfHeader);
	if (result) {
		goto CloseFile;
	}

	if (!MemoryCompare(&elfHeader.e_ident[EI_MAG0], ELFMAG, SELFMAG) || elfHeader.e_ident[EI_CLASS] != ELFCLASS64
		|| elfHeader.e_ident[EI_DATA] != ELFDATA2LSB || elfHeader.e_type != ET_EXEC || elfHeader.e_machine != EM_X86_64
		|| elfHeader.e_version != EV_CURRENT) {
		result = ResultSerialOutputUnavailable;
		goto CloseFile;
	}

	result = FileSetOffset(fileDescriptor, elfHeader.e_phoff);
	if (result) {
		goto CloseFile;
	}

	usz programHeaderTableSize = elfHeader.e_phnum * sizeof(Elf64_Phdr);
	void* programHeaderTablePage;
	result = AllocateBackedVirtualMemory(
		&g_kernelMemoryAllocator, Page4KiBNext(programHeaderTableSize), PageWriteable, &programHeaderTablePage);
	if (result) {
		goto CloseFile;
	}

	result = FileRead(fileDescriptor, programHeaderTableSize, programHeaderTablePage);
	if (result) {
		goto DeallocateProgramHeaderTable;
	}

	Elf64_Phdr* programHeaders = (Elf64_Phdr*)programHeaderTablePage;
	ELFSegmentRegion* regionBefore = nullptr;
	Elf64_Phdr* programHeaderBefore = nullptr;
	for (usz i = 0; i < elfHeader.e_phnum; ++i) {
		if (programHeaders[i].p_type != PT_LOAD)
			continue;

		// Rejecting any ELFs that have overlapping, out of order or incorrectly aligned segments
		if ((programHeaderBefore && programHeaderBefore->p_vaddr + programHeaderBefore->p_memsz >= programHeaders[i].p_vaddr)
			|| programHeaders[i].p_align != PAGE_4KIB_SIZE_BYTES) {
			result = ResultSerialOutputUnavailable;
			goto DeallocateProgramHeaderTable;
		}

		PageTableEntryFlags flags = 0;
		if (programHeaders[i].p_flags & PF_R)
			flags |= PagePresent;
		if (programHeaders[i].p_flags & PF_W)
			flags |= PageWriteable;
		if (!(programHeaders[i].p_flags & PF_X))
			flags |= PageNoExecute;

		Page4KiB begin = Page4KiBContaining(programHeaders[i].p_vaddr);
		Page4KiB end = Page4KiBNext(programHeaders[i].p_vaddr + programHeaders[i].p_memsz);

		if (regionBefore && regionBefore->End > begin) {
			regionBefore->End = end;
			regionBefore->Flags |= flags;
			continue;
		}

		ELFSegmentRegion* region;
		result = SizedBlockAllocate(&process->ELFSegmentMap, (void**)&region);
		if (result) {
			goto DeallocateProgramHeaderTable;
		}

		region->Begin = begin;
		region->End = end;
		region->Flags = flags;
		region->ELFSegment = programHeaders + i;

		regionBefore = region;
		programHeaderBefore = programHeaders + i;
	}

	ELFSegmentRegion* segmentRegionIter = nullptr;
	while (!SizedBlockIterate(&process->ELFSegmentMap, (void**)&segmentRegionIter)) {
		usz segmentRegionSize = segmentRegionIter->End - segmentRegionIter->Begin;

		void* segmentPage;
		result = AllocateBackedVirtualMemory(&g_kernelMemoryAllocator, segmentRegionSize, PageWriteable, &segmentPage);
		if (result) {
			goto DeallocateProgramHeaderTable;
		}

		MemoryFill(segmentPage, 0, segmentRegionSize);

		result = FileSetOffset(fileDescriptor, segmentRegionIter->ELFSegment->p_offset);
		if (result) {
			goto DeallocateProgramHeaderTable;
		}

		result = FileRead(fileDescriptor, segmentRegionIter->ELFSegment->p_filesz,
			((u8*)segmentPage + VirtualAddressPageOffset(segmentRegionIter->ELFSegment->p_vaddr)));
		if (result) {
			goto DeallocateProgramHeaderTable;
		}

		result = ReallocateVirtualMemory(&g_kernelMemoryAllocator, &process->VirtualMemoryAllocator, segmentRegionSize,
			segmentRegionIter->Flags | PageUserAccessible, (Page4KiB)segmentPage,
			Page4KiBContaining(segmentRegionIter->ELFSegment->p_vaddr));
		if (result) {
			goto DeallocateProgramHeaderTable;
		}
	}

	process->MainThread->Context.InterruptFrame.RIP = elfHeader.e_entry;

DeallocateProgramHeaderTable:
	DeallocateBackedVirtualMemory(&g_kernelMemoryAllocator, programHeaderTablePage, Page4KiBNext(programHeaderTableSize));
CloseFile:
	FileClose(&g_virtualFileSystem, fileDescriptor);
	return result;
}
