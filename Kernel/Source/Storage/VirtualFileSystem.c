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

Result MountpointCreate(VirtualFileSystem* fileSystem, i8 mountLetter, MountpointCapabilities capabilities, MountpointFunctions functions)
{
	Result result = ResultOk;

	if (mountLetter < 'A' || mountLetter > 'Z') {
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

Result MountpointDelete(VirtualFileSystem* fileSystem, i8 mountLetter)
{
	if (mountLetter < 'A' || mountLetter > 'Z' || !GetMountLetterStatus(fileSystem, mountLetter)) {
		return ResultSerialOutputUnavailable;
	}

	Mountpoint* mountpoint;
	Result result = MountpointGetFromLetter(fileSystem, mountLetter, &mountpoint);
	if (result) {
		return result;
	}

	result = SizedBlockDeallocate(&fileSystem->Mountpoints, mountpoint);
	if (result) {
		return result;
	}

	result = SetMountLetterStatus(fileSystem, mountLetter, false);
	if (result) {
		return result;
	}

	return result;
}

Result MountpointGetFromLetter(VirtualFileSystem* fileSystem, i8 mountLetter, Mountpoint** mountpoint)
{
	Mountpoint* mountpointIterator = nullptr;

	while (!SizedBlockIterate(&fileSystem->Mountpoints, (void**)&mountpointIterator)) {
		if (mountpointIterator->MountLetter != mountLetter) {
			continue;
		}

		*mountpoint = mountpointIterator;
		return ResultOk;
	}

	return ResultSerialOutputUnavailable;
}

Result SetMountLetterStatus(VirtualFileSystem* fileSystem, i8 mountLetter, bool reserved)
{
	if (mountLetter < 'A' || mountLetter > 'Z') {
		return ResultSerialOutputUnavailable;
	}

	const usz bit = mountLetter - 'A';

	if (reserved) {
		fileSystem->UsedMountLetters |= 1 << bit;
	} else {
		fileSystem->UsedMountLetters &= ~(1 << bit);
	}

	return ResultOk;
}

bool GetMountLetterStatus(VirtualFileSystem* fileSystem, i8 mountLetter)
{
	if (mountLetter < 'A' || mountLetter > 'Z') {
		return true;
	}

	const usz bit = mountLetter - 'A';

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
