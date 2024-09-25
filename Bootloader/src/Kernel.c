#include "Kernel.h"

#include "FrameAllocator.h"
#include "Logger.h"
#include "elf.h"
#include "Memory.h"

EFI_STATUS LoadKernel(
	UINT8* loadedFile,
	FrameAllocatorData* frameAllocator,
	EFI_PHYSICAL_ADDRESS p4TableAddress,
	EFI_VIRTUAL_ADDRESS* entryPoint)
{
	Elf64_Ehdr* elfHeader = (Elf64_Ehdr*) loadedFile;

	if(MemoryCompare(&elfHeader->e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0 ||
		elfHeader->e_ident[EI_CLASS] != ELFCLASS64 ||
		elfHeader->e_ident[EI_DATA] != ELFDATA2LSB ||
		elfHeader->e_type != ET_EXEC ||
		elfHeader->e_machine != EM_X86_64 ||
		elfHeader->e_version != EV_CURRENT)
    {
		SN_LOG_ERROR(L"The loaded kernel executable is either not an ELF64 file or is compiled to an unsupported format");
		return EFI_UNSUPPORTED;
    }

	Elf64_Phdr* programHeaders = (Elf64_Phdr*) (loadedFile + elfHeader->e_phoff);
	
	for(
		Elf64_Phdr* header = programHeaders;
        (UINT8*) header < (UINT8*) programHeaders + elfHeader->e_phnum * elfHeader->e_phentsize;
        header = (Elf64_Phdr*) ((UINT8*) header + elfHeader->e_phentsize))
	{
		if(header->p_type != PT_LOAD)
			continue;

		UINTN numPages = (header->p_memsz + 4095) / 4096;
		for(UINTN j = 0; j < numPages; j++)
		{
			// Allocate a memory frame, zero it out and copy the segments data to it
			// and map it to the kernel's P4 Table, if there's an error somewhere - shit yourself.
			EFI_PHYSICAL_ADDRESS frameAddress = 0;
			EFI_STATUS status = AllocateFrame(frameAllocator, &frameAddress);
			if(EFI_ERROR(status))
			{
				SN_LOG_ERROR(L"An unexpected error occured while trying to allocate a memory frame for the kernel");
				return status;
			}

			MemoryFill((VOID*) frameAddress, 0, 4096);
			MemoryCopy(loadedFile + header->p_offset + j * 4096, (VOID*) frameAddress, header->p_filesz);

			UINT64 flags = ENTRY_PRESENT;
			if(header->p_flags & PF_W)
				flags |= ENTRY_WRITEABLE;

			if(!(header->p_flags & PF_X))
				flags |= ENTRY_NO_EXECUTE;

			status = MapMemoryPage4KiB(header->p_vaddr + j * 4096, frameAddress, p4TableAddress, frameAllocator, flags);
			if(EFI_ERROR(status))
			{
				SN_LOG_ERROR(L"An unexpected error occured while trying to map a memory frame in the kernel's P4 table");
				return status;
			}
		}
	}

	*entryPoint = elfHeader->e_entry;

	return EFI_SUCCESS;
}

