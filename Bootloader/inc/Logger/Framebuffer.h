#pragma once

#include "Uefi.h"

/// The framebuffer logger's state.
typedef struct FramebufferLoggerData
{
	UINT32* framebuffer;
	UINTN   framebufferSize;
	UINTN   cursorPositionX;
	UINTN   cursorPositionY;
	UINTN   width;
	UINTN   height;
} FramebufferLoggerData;

/// Initializes the given framebuffer logger state with the first available Graphics Output Protocol available.
EFI_STATUS InitFramebufferLogger(EFI_SYSTEM_TABLE* systemTable, FramebufferLoggerData* logger);
/// Sets the given RGB value at the specified framebuffer position.
VOID FramebufferLoggerSetPixel(FramebufferLoggerData* logger, UINTN x, UINTN y, UINT8 red, UINT8 green, UINT8 blue);
/// Clears the entire framebuffer.
VOID FramebufferLoggerClear(FramebufferLoggerData* logger);
/// Writes a character to the framebuffer and increments the cursor position accordingly.
VOID FramebufferLoggerWriteChar(FramebufferLoggerData* logger, CHAR16 character);
/// Writes an entire string to the framebufer, incrementing the cursor position accordingly.
///
/// Note: The string has to be NULL-terminated.
VOID FramebufferLoggerWriteString(FramebufferLoggerData* logger, CHAR16* string);

