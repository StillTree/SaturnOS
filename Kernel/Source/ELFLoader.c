#include "ELFLoader.h"

#include "Logger.h"
#include "Memory.h"
#include "Memory/VirtualMemoryAllocator.h"
#include "Storage/VirtualFileSystem.h"
#include "elf.h"

static Result ELFDynamicSegment(Elf64_Phdr* progHeaders, usz progHeaderCount, Elf64_Phdr** dynamicSegment)
{
	for (usz i = 0; i < progHeaderCount; ++i) {
		if (progHeaders[i].p_type != PT_DYNAMIC)
			continue;

		*dynamicSegment = progHeaders + i;
		return ResultOk;
	}

	return ResultNotFound;
}

static VirtAddr ELFMinVirtAddr(Elf64_Phdr* programHeaders, usz count)
{
	VirtAddr minVirtAddr = U64_MAX;

	for (usz i = 0; i < count; ++i) {
		if (programHeaders[i].p_type != PT_LOAD)
			continue;

		if (programHeaders[i].p_vaddr < minVirtAddr) {
			minVirtAddr = programHeaders[i].p_vaddr;
		}
	}

	return minVirtAddr;
}

static Result ELFValidate(Elf64_Ehdr* elfHeader)
{
	if (!MemoryCompare(&elfHeader->e_ident[EI_MAG0], ELFMAG, SELFMAG) || elfHeader->e_ident[EI_CLASS] != ELFCLASS64
		|| elfHeader->e_ident[EI_DATA] != ELFDATA2LSB || elfHeader->e_machine != EM_X86_64 || elfHeader->e_version != EV_CURRENT) {
		return ResultSerialOutputUnavailable;
	}

	return ResultOk;
}

static Result ELFReadProgHeaderTable(Elf64_Ehdr* elfHeader, usz elfFile, Elf64_Phdr** progHeaderTable)
{
	Result result = FileSetOffset(elfFile, elfHeader->e_phoff);
	if (result) {
		return result;
	}

	usz progHeaderTableSize = elfHeader->e_phnum * sizeof(Elf64_Phdr);
	void* progHeaderTablePage;
	result = AllocateBackedVirtualMemory(&g_kernelMemoryAllocator, Page4KiBNext(progHeaderTableSize), PageWriteable, &progHeaderTablePage);
	if (result) {
		return result;
	}

	result = FileRead(elfFile, progHeaderTableSize, progHeaderTablePage);
	if (result) {
		DeallocateBackedVirtualMemory(&g_kernelMemoryAllocator, progHeaderTablePage, Page4KiBNext(progHeaderTableSize));
		return result;
	}

	*progHeaderTable = progHeaderTablePage;

	return result;
}

static Result ELFComputeRegions(Process* process, Elf64_Phdr* progHeaders, usz progHeaderCount, Page4KiB base)
{
	Result result = ResultOk;

	ELFSegmentRegion* regionBefore = nullptr;
	Elf64_Phdr* programHeaderBefore = nullptr;
	for (usz i = 0; i < progHeaderCount; ++i) {
		if (progHeaders[i].p_type != PT_LOAD)
			continue;

		// Rejecting any ELFs that have overlapping, out of order or incorrectly aligned segments
		if ((programHeaderBefore && programHeaderBefore->p_vaddr + programHeaderBefore->p_memsz >= progHeaders[i].p_vaddr)
			|| progHeaders[i].p_align != PAGE_4KIB_SIZE_BYTES) {
			return ResultSerialOutputUnavailable;
		}

		PageTableEntryFlags flags = 0;
		if (progHeaders[i].p_flags & PF_R)
			flags |= PagePresent;
		if (progHeaders[i].p_flags & PF_W)
			flags |= PageWriteable;
		// TODO: Fix this
		// if (!(programHeaders[i].p_flags & PF_X))
		// 	flags |= PageNoExecute;

		Page4KiB pageBegin = Page4KiBContaining(base + progHeaders[i].p_vaddr);
		Page4KiB pageEnd = Page4KiBNext(base + progHeaders[i].p_vaddr + progHeaders[i].p_memsz);

		if (regionBefore && regionBefore->End > pageBegin) {
			regionBefore->End = pageEnd;
			regionBefore->Flags |= flags;
			continue;
		}

		ELFSegmentRegion* region;
		result = SizedBlockAllocate(&process->ELFSegmentMap, (void**)&region);
		if (result) {
			return result;
		}

		region->Begin = pageBegin;
		region->End = pageEnd;
		region->Flags = flags;

		regionBefore = region;
		programHeaderBefore = progHeaders + i;
	}

	return result;
}

static Result ELFCopySegments(Process* process, usz elfFile, Elf64_Phdr* progHeaders, usz progHeaderCount, Page4KiB base)
{
	Result result = ResultOk;

	ELFSegmentRegion* segmentRegionIter = nullptr;
	while (!SizedBlockIterate(&process->ELFSegmentMap, (void**)&segmentRegionIter)) {
		result = AllocateBackedVirtualMemoryAtAddress(
			&process->VirtualMemoryAllocator, segmentRegionIter->End - segmentRegionIter->Begin, PageWriteable, segmentRegionIter->Begin);
		if (result) {
			return result;
		}
	}

	for (usz i = 0; i < progHeaderCount; ++i) {
		if (progHeaders[i].p_type != PT_LOAD)
			continue;

		void* segment = (u8*)progHeaders[i].p_vaddr + base;

		MemoryFill(segment, 0, progHeaders[i].p_memsz);

		result = FileSetOffset(elfFile, progHeaders[i].p_offset);
		if (result) {
			return result;
		}
		result = FileRead(elfFile, progHeaders[i].p_filesz, segment);
		if (result) {
			return result;
		}
	}

	return result;
}

static Result ELFApplyRelocations(Elf64_Phdr* dynamicSegment, Page4KiB base)
{
	Elf64_Dyn* relocationTag = (Elf64_Dyn*)(dynamicSegment->p_vaddr + base);

	VirtAddr relocationTableAddr = 0;
	usz relocationTableSize = 0;
	usz relocationEntrySize = 0;

	while (relocationTag->d_tag != DT_NULL) {
		switch (relocationTag->d_tag) {
		case DT_RELA:
			relocationTableAddr = relocationTag->d_un.d_ptr;
			break;
		case DT_RELASZ:
			relocationTableSize = relocationTag->d_un.d_val;
			break;
		case DT_RELAENT:
			relocationEntrySize = relocationTag->d_un.d_val;
			break;
		}

		++relocationTag;
	}

	if (relocationTableAddr) {
		usz relocationCount = relocationTableSize / relocationEntrySize;

		Elf64_Rela* relocationTable = (Elf64_Rela*)(base + relocationTableAddr);
		for (usz i = 0; i < relocationCount; ++i) {

			if (ELF64_R_TYPE(relocationTable[i].r_info) == R_X86_64_RELATIVE) {
				u64* relocation = (u64*)(base + relocationTable[i].r_offset);

				*relocation = base + relocationTable[i].r_addend;
			} else {
				LogLine(SK_LOG_WARN "An unknown relocation type encountered in: 0x%x", ELF64_R_TYPE(relocationTable[i].r_info));
				return ResultSerialOutputUnavailable;
			}
		}
	}

	return ResultOk;
}

static Result ELFRemapSegments(Process* process)
{
	Result result = ResultOk;

	ELFSegmentRegion* segmentRegionIter = nullptr;
	while (!SizedBlockIterate(&process->ELFSegmentMap, (void**)&segmentRegionIter)) {
		result = RemapVirtualMemory(&process->VirtualMemoryAllocator, segmentRegionIter->Begin,
			segmentRegionIter->End - segmentRegionIter->Begin, segmentRegionIter->Flags | PageUserAccessible);
		if (result) {
			return result;
		}
	}

	return result;
}

static Result ELFLoadDYN(Process* process, usz elfFile, Elf64_Ehdr* elfHeader, Elf64_Phdr* progHeaders, usz progHeaderCount)
{
	// TODO: Randomise base
	Page4KiB base = 0x400000;
	VirtAddr minVirtAddr = ELFMinVirtAddr(progHeaders, progHeaderCount);
	Page4KiB minPage = Page4KiBContaining(minVirtAddr);
	base -= minPage;

	Result result = ELFComputeRegions(process, progHeaders, progHeaderCount, base);
	if (result) {
		return result;
	}

	result = ELFCopySegments(process, elfFile, progHeaders, progHeaderCount, base);
	if (result) {
		return result;
	}

	Elf64_Phdr* dynamicSegment = nullptr;
	result = ELFDynamicSegment(progHeaders, progHeaderCount, &dynamicSegment);
	if (result) {
		return result;
	}

	if (dynamicSegment) {
		result = ELFApplyRelocations(dynamicSegment, base);
		if (result) {
			return result;
		}
	}

	result = ELFRemapSegments(process);
	if (result) {
		return result;
	}

	process->MainThread->Context.InterruptFrame.RIP = elfHeader->e_entry + base;

	return ResultOk;
}

static Result ELFLoadEXEC(Process* process, usz elfFile, Elf64_Ehdr* elfHeader, Elf64_Phdr* progHeaders, usz progHeaderCount)
{
	Result result = ELFComputeRegions(process, progHeaders, progHeaderCount, 0);
	if (result) {
		return result;
	}

	result = ELFCopySegments(process, elfFile, progHeaders, progHeaderCount, 0);
	if (result) {
		return result;
	}

	result = ELFRemapSegments(process);
	if (result) {
		return result;
	}

	process->MainThread->Context.InterruptFrame.RIP = elfHeader->e_entry;

	return ResultOk;
}

Result ProcessLoadELF(Process* process, const i8* elfPath)
{
	usz fileDescriptor;
	Result result = FileOpen(&g_virtualFileSystem, elfPath, OpenFileRead, &fileDescriptor);
	if (result) {
		return result;
	}

	Elf64_Ehdr elfHeader;
	result = FileRead(fileDescriptor, sizeof elfHeader , &elfHeader);
	if (result) {
		goto CloseFile;
	}

	result = ELFValidate(&elfHeader);
	if (result) {
		goto CloseFile;
	}

	Elf64_Phdr* progHeaders;
	result = ELFReadProgHeaderTable(&elfHeader, fileDescriptor, &progHeaders);
	if (result) {
		goto CloseFile;
	}

	ProcessStepInto(process);
	if (elfHeader.e_type == ET_EXEC) {
		result = ELFLoadEXEC(process, fileDescriptor, &elfHeader, progHeaders, elfHeader.e_phnum);
	} else if (elfHeader.e_type == ET_DYN) {
		result = ELFLoadDYN(process, fileDescriptor, &elfHeader, progHeaders, elfHeader.e_phnum);
	} else {
		result = ResultSerialOutputUnavailable;
	}
	ProcessStepOut();

	DeallocateBackedVirtualMemory(&g_kernelMemoryAllocator, progHeaders, Page4KiBNext(sizeof(Elf64_Phdr) * elfHeader.e_phnum));
CloseFile:
	FileClose(&g_virtualFileSystem, fileDescriptor);

	return result;
}
