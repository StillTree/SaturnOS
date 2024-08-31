#pragma once

#include "UefiTypes.h"

#define EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID \
	{ \
		0x387477c2, 0x69c7, 0x11d2, {0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b } \
	}

typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

/**
	Write a string to the output device.

	@param  This   The protocol instance pointer.
	@param  String The NULL-terminated string to be displayed on the output
	               device(s). All output devices must also support the Unicode
	               drawing character codes defined in this file.

	@retval EFI_SUCCESS             The string was output to the device.
	@retval EFI_DEVICE_ERROR        The device reported an error while attempting to output
	                                the text.
	@retval EFI_UNSUPPORTED         The output device's mode is not currently in a
	                                defined text mode.
	@retval EFI_WARN_UNKNOWN_GLYPH  This warning code indicates that some of the
	                                characters in the string could not be
	                                rendered and were skipped.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_TEXT_STRING)(
	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
	IN CHAR16                          *String
);

/**
	Clears the output device(s) display to the currently selected background
	color.

	@param  This              The protocol instance pointer.

	@retval  EFI_SUCCESS      The operation completed successfully.
	@retval  EFI_DEVICE_ERROR The device had an error and could not complete the request.
	@retval  EFI_UNSUPPORTED  The output device is not in a valid text mode.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_TEXT_CLEAR_SCREEN)(
	IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This
);

///
/// The SIMPLE_TEXT_OUTPUT protocol is used to control text-based output devices.
/// It is the minimum required protocol for any handle supplied as the ConsoleOut
/// or StandardError device. In addition, the minimum supported text mode of such
/// devices is at least 80 x 25 characters.
///
struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
	VOID* Reset;

	EFI_TEXT_STRING OutputString;
	VOID*           TestString;

	VOID* QueryMode;
	VOID* SetMode;
	VOID* SetAttribute;

	EFI_TEXT_CLEAR_SCREEN ClearScreen;
	VOID*                 SetCursorPosition;
	VOID*                 EnableCursor;

	///
	/// Pointer to SIMPLE_TEXT_OUTPUT_MODE data.
	///
	VOID* Mode;
};

extern EFI_GUID gEfiSimpleTextOutProtocolGuid;

