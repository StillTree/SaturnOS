#include "EspFileSystem.h"

#include "Logger.h"

EFI_STATUS ReadFile(EFI_SYSTEM_TABLE* systemTable, EFI_FILE_PROTOCOL* rootVolume, CHAR16* fileName, VOID** fileBuffer, UINTN* fileSizeBytes)
{
	// Firstly open the file
	EFI_FILE_PROTOCOL* openedFile = NULL;
	EFI_STATUS status = rootVolume->Open(rootVolume, &openedFile, fileName, EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR(status)) {
		SN_LOG_ERROR(L"An unexpected error occured while trying to open a file");
		return status;
	}

	// A memory pool needs to be allocated for the file information structure so we get its size
	EFI_GUID efiInfoId = EFI_FILE_INFO_ID;
	UINTN fileInfoSize = 0;
	EFI_FILE_INFO* fileInfo = NULL;
	status = openedFile->GetInfo(openedFile, &efiInfoId, &fileInfoSize, fileInfo);
	if (EFI_ERROR(status) && status != EFI_BUFFER_TOO_SMALL) {
		SN_LOG_ERROR(L"An unexpected error occured while trying to get the file information buffer size");
		goto closeFile;
	}

	status = systemTable->BootServices->AllocatePool(EfiLoaderData, fileInfoSize, (VOID**)&fileInfo);
	if (EFI_ERROR(status)) {
		SN_LOG_ERROR(L"An unexpected error occured while trying to allocate a memory pool for the file information buffer");
		goto closeFile;
	}

	// After allocating the correct memory size returned from an "error" we write data into it
	status = openedFile->GetInfo(openedFile, &efiInfoId, &fileInfoSize, fileInfo);
	if (EFI_ERROR(status)) {
		SN_LOG_ERROR(L"An unexpected error occured while trying to get the file information");
		goto freeFileInfo;
	}

	if (fileSizeBytes) {
		*fileSizeBytes = fileInfo->FileSize;
	}

	// With the file size correctly known we can allocate a memory pool for the file itself and read it into the allocated pool
	status = systemTable->BootServices->AllocatePool(EfiLoaderData, fileInfo->FileSize, fileBuffer);
	if (EFI_ERROR(status)) {
		SN_LOG_ERROR(L"An unexpected error occured while trying to allocate a memory pool for the file contents");
		goto freeFileInfo;
	}

	UINTN fileSize = fileInfo->FileSize;
	status = openedFile->Read(openedFile, &fileSize, *fileBuffer);
	if (EFI_ERROR(status)) {
		SN_LOG_ERROR(L"An unexpected error occured while trying to read the file contents");
		systemTable->BootServices->FreePool(*fileBuffer);
		goto closeFile;
	}

freeFileInfo:
	systemTable->BootServices->FreePool(fileInfo);

closeFile:
	openedFile->Close(openedFile);

	return status;
}
