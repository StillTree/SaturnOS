#include "EspFileSystem.h"

#include "Logger.h"

EFI_STATUS ReadFile(EFI_SYSTEM_TABLE* systemTable, EFI_FILE_PROTOCOL* rootVolume, CHAR16* fileName, VOID** fileBuffer)
{
	EFI_FILE_PROTOCOL* openedFile = NULL;
	EFI_STATUS status = rootVolume->Open(rootVolume, &openedFile, fileName, EFI_FILE_MODE_READ, 0);
	if(EFI_ERROR(status))
	{
		SN_LOG_ERROR(L"An unexpected error occured while trying to open a file");
		return status;
	}

	UINTN fileSize = 0;
	status = openedFile->Read(
		openedFile,
		&fileSize,
		NULL);
	if(EFI_ERROR(status) && status != EFI_BUFFER_TOO_SMALL)
	{
		SN_LOG_ERROR(L"An unexpected error occured while trying to get the file size");
		goto closeFile;
	}

	status = systemTable->BootServices->AllocatePool(
		EfiLoaderData,
		fileSize,
		fileBuffer);
	if(EFI_ERROR(status))
	{
		SN_LOG_ERROR(L"An unexpected error occured while trying to allocate a memory pool for the file contents");
		goto closeFile;
	}

	status = openedFile->Read(
		openedFile,
		&fileSize,
		*fileBuffer);
	if(EFI_ERROR(status))
	{
		SN_LOG_ERROR(L"An unexpected error occured while trying to read the file contents");
		systemTable->BootServices->FreePool(*fileBuffer);
		goto closeFile;
	}

closeFile:
	openedFile->Close(openedFile);

	return status;
}

