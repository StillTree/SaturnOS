#include "Storage/VirtualFileSystem.h"

#include "Memory/Page.h"
#include "Memory/PageTable.h"
#include "Memory/VirtualMemoryAllocator.h"

VirtualFileSystem g_virtualFileSystem;

Result InitVirtualFileSystem(VirtualFileSystem* fileSystem)
{
	usz mountpointPoolSize = Page4KiBNext(sizeof(Mountpoint) * MAX_MOUNTPOINTS);
	Page4KiB mountpointPool;
	Result result = AllocateBackedVirtualMemory(&g_kernelMemoryAllocator, mountpointPoolSize, PageWriteable, &mountpointPool);
	if (result) {
		return result;
	}

	result = InitSizedBlockAllocator(&fileSystem->Mountpoints, mountpointPool, mountpointPoolSize, sizeof(Mountpoint));
	if (result) {
		return result;
	}

	usz openedFilesPoolSize = Page4KiBNext(sizeof(OpenedFile) * MAX_OPENED_FILES);
	Page4KiB openedFilesPool;
	result = AllocateBackedVirtualMemory(&g_kernelMemoryAllocator, openedFilesPoolSize, PageWriteable, &openedFilesPool);
	if (result) {
		return result;
	}

	result = InitSizedBlockAllocator(&fileSystem->OpenedFiles, openedFilesPool, openedFilesPoolSize, sizeof(OpenedFile));
	if (result) {
		return result;
	}

	fileSystem->UsedMountLetters = 0;

	return result;
}

Result CreateMountpoint(VirtualFileSystem* fileSystem, i8 mountLetter, MountpointCapabilities capabilities, MountpointFunctions functions)
{
	Result result = ResultOk;

	if (mountLetter < 65 || mountLetter > 90) {
		result = GetFirstUnusedMountLetter(fileSystem, &mountLetter);
		if (result) {
			return result;
		}
	} else if (GetMountLetterStatus(fileSystem, mountLetter)) {
		return ResultSerialOutputUnavailable;
	}

	result = SetMountLetterStatus(fileSystem, mountLetter, true);
	if (result) {
		return result;
	}

	Mountpoint* mountpoint;
	result = SizedBlockAllocate(&fileSystem->Mountpoints, (void**)&mountpoint);
	if (result) {
		return result;
	}

	mountpoint->MountLetter = mountLetter;
	mountpoint->Capabilities = capabilities;
	mountpoint->Functions = functions;

	return result;
}

Result DeleteMountpoint(VirtualFileSystem* fileSystem, i8 mountLetter)
{
	if (mountLetter < 65 || mountLetter > 90 || !GetMountLetterStatus(fileSystem, mountLetter)) {
		return ResultSerialOutputUnavailable;
	}

	Result result = SetMountLetterStatus(fileSystem, mountLetter, false);
	if (result) {
		return result;
	}



	return ResultOk;
}

Result SetMountLetterStatus(VirtualFileSystem* fileSystem, i8 mountLetter, bool reserved)
{
	if (mountLetter < 65 || mountLetter > 90) {
		return ResultSerialOutputUnavailable;
	}

	const usz bit = mountLetter - 65;

	if (reserved) {
		fileSystem->UsedMountLetters |= 1 << bit;
	} else {
		fileSystem->UsedMountLetters &= ~(1 << bit);
	}

	return ResultOk;
}

bool GetMountLetterStatus(VirtualFileSystem* fileSystem, i8 mountLetter)
{
	if (mountLetter < 65 || mountLetter > 90) {
		return true;
	}

	const usz bit = mountLetter - 65;

	return ((fileSystem->UsedMountLetters >> bit) & 1) == 1;
}

Result GetFirstUnusedMountLetter(VirtualFileSystem* fileSystem, i8* mountLetter)
{
	for (i8 i = 'A'; i <= (i8)'Z'; ++i) {
		if (GetMountLetterStatus(fileSystem, i)) {
			continue;
		}

		*mountLetter = i;
		return ResultOk;
	}

	return ResultSerialOutputUnavailable;
}
