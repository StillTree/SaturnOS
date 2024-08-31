#include "Uefi.h"

EFI_STATUS EFIAPI UefiMain(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable)
{
	systemTable->ConOut->ClearScreen(systemTable->ConOut);

	systemTable->ConOut->OutputString(systemTable->ConOut, L"Zupa obiad\r\n");

	while(1);

	return EFI_SUCCESS;
}
