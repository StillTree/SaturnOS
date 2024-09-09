#include "Logger/Framebuffer.h"

#include "Memory.h"

EFI_STATUS InitFramebufferLogger(EFI_SYSTEM_TABLE* systemTable, FramebufferLoggerData* logger)
{
	systemTable->ConOut->OutputString(
		systemTable->ConOut,
		L"Initializing the framebuffer logger... ");

	EFI_STATUS status = EFI_SUCCESS;

	EFI_GRAPHICS_OUTPUT_PROTOCOL* graphicsOutputProtocol = NULL;
	status = systemTable->BootServices->LocateProtocol(
		&gEfiGraphicsOutputProtocolGuid,
		NULL,
		(VOID**) &graphicsOutputProtocol);
	if(EFI_ERROR(status))
	{
		systemTable->ConOut->OutputString(
			systemTable->ConOut,
			L"An unexpected error occured while trying to open the Graphics Output Protocol");
		return status;
	}

	INTN suitableModeIndex = -1;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* suitableModeInfo = NULL;
	for(UINTN i = 0; i < graphicsOutputProtocol->Mode->MaxMode; i++)
	{
		EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info = NULL;
		UINTN infoSize = 0;
		status = graphicsOutputProtocol->QueryMode(
			graphicsOutputProtocol,
			i,
			&infoSize,
			&info);
		if(EFI_ERROR(status))
		{
			systemTable->ConOut->OutputString(
				systemTable->ConOut,
				L"An unexpected error occured while trying to query the Graphics Output Protocol mode information");
			return status;
		}

		if(info->HorizontalResolution >= 800
			&& info->VerticalResolution >= 600
			&& (info->PixelFormat == PixelRedGreenBlueReserved8BitPerColor || info->PixelFormat == PixelBlueGreenRedReserved8BitPerColor))
		{
			suitableModeInfo  = info;
			suitableModeIndex = i;
			break;
		}
		
		systemTable->BootServices->FreePool(info);
	}

	// If both of these checks aren't true something truly terrible went wrong 💀
	if(!suitableModeInfo || suitableModeIndex == -1)
	{
		systemTable->ConOut->OutputString(
			systemTable->ConOut,
			L"Could not find any suitable 800x600 Graphics Output modes");
		return EFI_UNSUPPORTED;
	}

	status = graphicsOutputProtocol->SetMode(graphicsOutputProtocol, suitableModeIndex);
	if(EFI_ERROR(status))
	{
		systemTable->ConOut->OutputString(
			systemTable->ConOut,
			L"An unexpected error occured while trying to set a Graphics Output mode");
		return status;
	}

	logger->framebuffer     = (UINT32*) graphicsOutputProtocol->Mode->FrameBufferBase;
	logger->framebufferSize = graphicsOutputProtocol->Mode->FrameBufferSize;
	logger->cursorPositionX = 0;
	logger->cursorPositionY = 0;
	logger->width           = graphicsOutputProtocol->Mode->Info->HorizontalResolution;
	logger->height          = graphicsOutputProtocol->Mode->Info->VerticalResolution;

	systemTable->BootServices->FreePool(suitableModeInfo);

	// TODO: Close the GOP
	//
	systemTable->ConOut->OutputString(
		systemTable->ConOut,
		L"Done\r\n");

	return status;
}

VOID FramebufferLoggerClear(FramebufferLoggerData *logger)
{
	MemoryFill(logger->framebuffer, 0, logger->framebufferSize);

	logger->cursorPositionY = 0;
	logger->cursorPositionX = 0;
}

VOID FramebufferLoggerSetPixel(FramebufferLoggerData* logger, UINTN x, UINTN y, UINT8 red, UINT8 green, UINT8 blue)
{
	logger->framebuffer[y * logger->width + x] = (red << 16) | (green << 8) | blue;
}

extern const CHAR16 g_fontBitmaps[96][20][10];

VOID FramebufferLoggerWriteChar(FramebufferLoggerData* logger, CHAR16 character)
{
	UINTN charIndex = character > 126 ? 95 : character - 32;

	if(character == L'\n')
	{
		if(logger->cursorPositionY + 40 >= logger->height)
		{
			FramebufferLoggerClear(logger);
			return;
		}

		logger->cursorPositionY += 20;
		logger->cursorPositionX = 0;
		return;
	}

	if(character == L'\r')
	{
		logger->cursorPositionX = 0;
		return;
	}

	if(logger->cursorPositionX + 9 >= logger->width)
	{
		FramebufferLoggerWriteChar(logger, L'\n');
		return;
	}

	for(UINTN y = 0; y < 20; y++)
	{
		for(UINTN x = 0; x < 10; x++)
		{
			FramebufferLoggerSetPixel(
				logger,
				x + logger->cursorPositionX,
				y + logger->cursorPositionY,
				g_fontBitmaps[charIndex][y][x],
				g_fontBitmaps[charIndex][y][x],
				g_fontBitmaps[charIndex][y][x]);
		}
	}

	logger->cursorPositionX += 9;
}

VOID FramebufferLoggerWriteString(FramebufferLoggerData* logger, CHAR16* string)
{
	UINTN i = 0;
	while(string[i])
	{
		FramebufferLoggerWriteChar(logger, string[i++]);
	}
}

