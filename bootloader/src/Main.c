#include "Uefi.h"

#include "Logger/Framebuffer.h"

EFI_GUID gEfiGraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
EFI_GUID gEfiSimpleTextOutProtocolGuid  = EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID;

EFI_STATUS EFIAPI UefiMain(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable)
{
	(void) imageHandle;

	systemTable->ConOut->ClearScreen(systemTable->ConOut);

	FramebufferLoggerData zupa = { NULL, 0, 0, 0, 0, 0 };

	InitFramebufferLogger(systemTable, &zupa);

	FramebufferLoggerWriteString(&zupa, L"żołądź~!\n");

halt:
	while(TRUE)
	{
		__asm__("cli; hlt");
	}

	// We never ever want to get here
	return EFI_SUCCESS;
}

