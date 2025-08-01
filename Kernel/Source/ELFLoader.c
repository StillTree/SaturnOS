#include "ELFLoader.h"

#include "Memory.h"
#include "Memory/VirtualMemoryAllocator.h"
#include "Storage/VirtualFileSystem.h"
#include "elf.h"

static Result MapELFSegment(Process* process, usz fileDescriptor, const Elf64_Phdr* programHeader)
{
	PageTableEntryFlags flags = 0;
	if (programHeader->p_flags & PF_R)
		flags |= PagePresent;
	if (programHeader->p_flags & PF_W)
		flags |= PageWriteable;
	if (!(programHeader->p_flags & PF_X))
		flags |= PageNoExecute;

	Page4KiB segmentPage;
	Result result
		= AllocateBackedVirtualMemory(&g_kernelMemoryAllocator, Page4KiBNext(programHeader->p_memsz), PageWriteable, &segmentPage);
	if (result) {
		return result;
	}

	MemoryFill((void*)segmentPage, 0, Page4KiBNext(programHeader->p_memsz));

	result = FileSetOffset(fileDescriptor, programHeader->p_offset);
	if (result) {
		return result;
	}

	result = FileRead(fileDescriptor, programHeader->p_filesz, (void*)segmentPage);
	if (result) {
		return result;
	}

	return ReallocateVirtualMemory(&g_kernelMemoryAllocator, &process->VirtualMemoryAllocator, Page4KiBNext(programHeader->p_memsz),
		flags | PageUserAccessible, segmentPage, programHeader->p_vaddr);
}

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
	Page4KiB programHeaderTablePage;
	result = AllocateBackedVirtualMemory(
		&g_kernelMemoryAllocator, Page4KiBNext(programHeaderTableSize), PageWriteable, &programHeaderTablePage);
	if (result) {
		goto CloseFile;
	}

	result = FileRead(fileDescriptor, programHeaderTableSize, (void*)programHeaderTablePage);
	if (result) {
		goto DeallocateMemory;
	}

	Elf64_Phdr* programHeaders = (Elf64_Phdr*)programHeaderTablePage;
	for (usz i = 0; i < elfHeader.e_phnum; ++i) {
		if (programHeaders[i].p_type != PT_LOAD)
			continue;

		result = MapELFSegment(process, fileDescriptor, &programHeaders[i]);
		if (result) {
			goto DeallocateMemory;
		}
	}

	process->MainThread->Context.InterruptFrame.RIP = elfHeader.e_entry;

DeallocateMemory:
	DeallocateBackedVirtualMemory(&g_kernelMemoryAllocator, programHeaderTablePage, Page4KiBNext(programHeaderTableSize));
CloseFile:
	FileClose(&g_virtualFileSystem, fileDescriptor);
	return result;
}
