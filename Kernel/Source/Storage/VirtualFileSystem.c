#include "Storage/VirtualFileSystem.h"

#include "Memory/Page.h"
#include "Memory/PageTable.h"
#include "Memory/VirtualMemoryAllocator.h"
#include "Scheduler.h"

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

	return ResultNotFound;
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

static Result GetMatchingFile(VirtualFileSystem* fileSystem, Mountpoint* mountpoint, u64 id, OpenedFile** openedFile)
{
	OpenedFile* openedFileIterator = nullptr;

	while (!SizedBlockIterate(&fileSystem->OpenedFiles, (void**)&openedFileIterator)) {
		if (openedFileIterator->Mountpoint != mountpoint || openedFileIterator->CachedInformation.ID != id) {
			continue;
		}

		*openedFile = openedFileIterator;
		return ResultOk;
	}

	return ResultNotFound;
}

Result FileOpen(VirtualFileSystem* fileSystem, const i8* path, OpenedFileMode mode, ProcessFileDescriptor** fileDescriptor)
{
	// TODO: Make this not shit
	if (!path || path[0] < 'A' || path[0] > 'Z' || path[1] != ':' || path[2] != '/') {
		return ResultSerialOutputUnavailable;
	}

	Mountpoint* mountpoint = nullptr;
	Result result = MountpointGetFromLetter(fileSystem, path[0], &mountpoint);
	if (result) {
		return result;
	}

	if (mode & OpenFileWrite && !(mountpoint->Capabilities & MountpointWriteable)) {
		return ResultSerialOutputUnavailable;
	}

	if (mode & OpenFileRead && !(mountpoint->Capabilities & MountpointReadable)) {
		return ResultSerialOutputUnavailable;
	}

	u64 fileID;
	result = mountpoint->Functions.FileLookupID(path + 2, &fileID);
	if (result) {
		return result;
	}

	bool matchingFileFound = true;
	OpenedFile* openedFile = nullptr;
	result = GetMatchingFile(fileSystem, mountpoint, fileID, &openedFile);
	if (result) {
		// The file is not opened by any other process, let's allocate a new opened file
		result = SizedBlockAllocate(&fileSystem->OpenedFiles, (void**)&openedFile);
		if (result) {
			return result;
		}

		openedFile->Mountpoint = mountpoint;
		openedFile->References = 0;
		matchingFileFound = false;
	}

	result = SizedBlockAllocate(&g_scheduler.CurrentThread->ParentProcess->FileDescriptors, (void**)fileDescriptor);
	if (result) {
		goto DeallocateOpenedFile;
	}

	(*fileDescriptor)->OpenedFile = openedFile;
	(*fileDescriptor)->OffsetBytes = 0;

	++openedFile->References;

	// TODO: In the future add an `OpenedFileMode` that will instead create that file in the case that it doesn't exist
	result = mountpoint->Functions.FileOpen(path + 2, &openedFile->FileSystemSpecific);
	if (result) {
		goto DeallocateFileDescriptor;
	}

	result = mountpoint->Functions.FileInformation(openedFile->FileSystemSpecific, &openedFile->CachedInformation);
	if (result) {
		goto CloseFile;
	}

	return result;

CloseFile:
	mountpoint->Functions.FileClose(openedFile->FileSystemSpecific);
DeallocateFileDescriptor:
	SizedBlockDeallocate(&g_scheduler.CurrentThread->ParentProcess->FileDescriptors, *fileDescriptor);
DeallocateOpenedFile:
	if (!matchingFileFound) {
		SizedBlockDeallocate(&fileSystem->OpenedFiles, openedFile);
	}

	return result;
}

Result FileRead(ProcessFileDescriptor* fileDescriptor, usz countBytes, void* buffer)
{
	if (!fileDescriptor) {
		return ResultSerialOutputUnavailable;
	}

	usz fileDescriptorIndex = SizedBlockGetIndex(&g_scheduler.CurrentThread->ParentProcess->FileDescriptors, fileDescriptor);
	if (!SizedBlockGetStatus(&g_scheduler.CurrentThread->ParentProcess->FileDescriptors, fileDescriptorIndex)) {
		return ResultSerialOutputUnavailable;
	}

	Mountpoint* mountpoint = fileDescriptor->OpenedFile->Mountpoint;

	if (countBytes + fileDescriptor->OffsetBytes > fileDescriptor->OpenedFile->CachedInformation.Size) {
		countBytes = fileDescriptor->OpenedFile->CachedInformation.Size - fileDescriptor->OffsetBytes;
	}

	Result result
		= mountpoint->Functions.FileRead(fileDescriptor->OpenedFile->FileSystemSpecific, fileDescriptor->OffsetBytes, countBytes, buffer);
	if (result) {
		return result;
	}

	fileDescriptor->OffsetBytes += countBytes;

	return result;
}

Result FileInformation(ProcessFileDescriptor* fileDescriptor, OpenedFileInformation* fileInformation)
{
	if (!fileDescriptor) {
		return ResultSerialOutputUnavailable;
	}

	usz fileDescriptorIndex = SizedBlockGetIndex(&g_scheduler.CurrentThread->ParentProcess->FileDescriptors, fileDescriptor);
	if (!SizedBlockGetStatus(&g_scheduler.CurrentThread->ParentProcess->FileDescriptors, fileDescriptorIndex)) {
		return ResultSerialOutputUnavailable;
	}

	Mountpoint* mountpoint = fileDescriptor->OpenedFile->Mountpoint;

	Result result = mountpoint->Functions.FileInformation(fileDescriptor->OpenedFile->FileSystemSpecific, fileInformation);
	if (result) {
		return result;
	}

	fileDescriptor->OpenedFile->CachedInformation = *fileInformation;

	return result;
}

Result FileSetOffset(ProcessFileDescriptor* fileDescriptor, usz offset)
{
	// TODO: Remove later when adding writing to files
	if (offset >= fileDescriptor->OpenedFile->CachedInformation.Size) {
		return ResultSerialOutputUnavailable;
	}

	fileDescriptor->OffsetBytes = offset;

	return ResultOk;
}

Result FileClose(VirtualFileSystem* fileSystem, ProcessFileDescriptor* fileDescriptor)
{
	if (!fileDescriptor) {
		return ResultSerialOutputUnavailable;
	}

	Result result = ResultOk;

	if (fileDescriptor->OpenedFile->References <= 1) {
		// This descriptor is the only one referencing this file, deallocate everything
		result = fileDescriptor->OpenedFile->Mountpoint->Functions.FileClose(fileDescriptor->OpenedFile->FileSystemSpecific);
		if (result) {
			return result;
		}

		result = SizedBlockDeallocate(&fileSystem->OpenedFiles, fileDescriptor->OpenedFile);
		if (result) {
			return result;
		}

		result = SizedBlockDeallocate(&g_scheduler.CurrentThread->ParentProcess->FileDescriptors, fileDescriptor);
		if (result) {
			return result;
		}
	} else {
		// There are other processes referencing this file, deallocate only current process's file descriptor
		--fileDescriptor->OpenedFile->References;

		result = SizedBlockDeallocate(&g_scheduler.CurrentThread->ParentProcess->FileDescriptors, fileDescriptor);
		if (result) {
			return result;
		}
	}

	return result;
}
