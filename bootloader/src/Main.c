#include "Uefi.h"
#include "Logger.h"

EFI_GUID gEfiGraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
EFI_GUID gEfiSimpleTextOutProtocolGuid  = EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID;

EFI_STATUS EFIAPI UefiMain(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable)
{
	(void) imageHandle;
	EFI_STATUS status = EFI_SUCCESS;

	status = InitLogger(systemTable, &g_mainLogger, TRUE, TRUE);
	if(EFI_ERROR(status))
	{
		goto halt;
	}

halt:
	while(TRUE)
	{
		__asm__("cli; hlt");
	}

	// We never ever want to get here
	return EFI_SUCCESS;
}

