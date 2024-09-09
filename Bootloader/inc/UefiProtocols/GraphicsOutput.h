#pragma once

#include "UefiTypes.h"

#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID \
	{ \
		0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a } \
	}

typedef struct _EFI_GRAPHICS_OUTPUT_PROTOCOL EFI_GRAPHICS_OUTPUT_PROTOCOL;

typedef struct {
	UINT32 RedMask;
	UINT32 GreenMask;
	UINT32 BlueMask;
	UINT32 ReservedMask;
} EFI_PIXEL_BITMASK;

typedef enum {
	///
	/// A pixel is 32-bits and byte zero represents red, byte one represents green,
	/// byte two represents blue, and byte three is reserved. This is the definition
	/// for the physical frame buffer. The byte values for the red, green, and blue
	/// components represent the color intensity. This color intensity value range
	/// from a minimum intensity of 0 to maximum intensity of 255.
	///
	PixelRedGreenBlueReserved8BitPerColor,
	///
	/// A pixel is 32-bits and byte zero represents blue, byte one represents green,
	/// byte two represents red, and byte three is reserved. This is the definition
	/// for the physical frame buffer. The byte values for the red, green, and blue
	/// components represent the color intensity. This color intensity value range
	/// from a minimum intensity of 0 to maximum intensity of 255.
	///
	PixelBlueGreenRedReserved8BitPerColor,
	///
	/// The Pixel definition of the physical frame buffer.
	///
	PixelBitMask,
	///
	/// This mode does not support a physical frame buffer.
	///
	PixelBltOnly,
	///
	/// Valid EFI_GRAPHICS_PIXEL_FORMAT enum values are less than this value.
	///
	PixelFormatMax
} EFI_GRAPHICS_PIXEL_FORMAT;

typedef struct {
	///
	/// The version of this data structure. A value of zero represents the
	/// EFI_GRAPHICS_OUTPUT_MODE_INFORMATION structure as defined in this specification.
	///
	UINT32 Version;
	///
	/// The size of video screen in pixels in the X dimension.
	///
	UINT32 HorizontalResolution;
	///
	/// The size of video screen in pixels in the Y dimension.
	///
	UINT32 VerticalResolution;
	///
	/// Enumeration that defines the physical format of the pixel. A value of PixelBltOnly
	/// implies that a linear frame buffer is not available for this mode.
	///
	EFI_GRAPHICS_PIXEL_FORMAT PixelFormat;
	///
	/// This bit-mask is only valid if PixelFormat is set to PixelPixelBitMask.
	/// A bit being set defines what bits are used for what purpose such as Red, Green, Blue, or Reserved.
	///
	EFI_PIXEL_BITMASK PixelInformation;
	///
	/// Defines the number of pixel elements per video memory line.
	///
	UINT32 PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

/**
	Returns information for an available graphics mode that the graphics device
	and the set of active video output devices supports.

	@param  This                  The EFI_GRAPHICS_OUTPUT_PROTOCOL instance.
	@param  ModeNumber            The mode number to return information on.
  	@param  SizeOfInfo            A pointer to the size, in bytes, of the Info buffer.
	@param  Info                  A pointer to callee allocated buffer that returns information about ModeNumber.

	@retval EFI_SUCCESS           Valid mode information was returned.
	@retval EFI_DEVICE_ERROR      A hardware error occurred trying to retrieve the video mode.
	@retval EFI_INVALID_PARAMETER ModeNumber is not valid.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE)(
	IN  EFI_GRAPHICS_OUTPUT_PROTOCOL         *This,
	IN  UINT32                               ModeNumber,
	OUT UINTN                                *SizeOfInfo,
	OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **Info
);

/**
	Set the video device into the specified mode and clears the visible portions of
	the output display to black.

	@param  This              The EFI_GRAPHICS_OUTPUT_PROTOCOL instance.
	@param  ModeNumber        Abstraction that defines the current video mode.

	@retval EFI_SUCCESS       The graphics mode specified by ModeNumber was selected.
	@retval EFI_DEVICE_ERROR  The device had an error and could not complete the request.
	@retval EFI_UNSUPPORTED   ModeNumber is not supported by this device.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE)(
	IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
	IN UINT32                       ModeNumber
);

typedef struct {
	///
	/// The number of modes supported by QueryMode() and SetMode().
	///
	UINT32 MaxMode;
	///
	/// Current Mode of the graphics device. Valid mode numbers are 0 to MaxMode -1.
	///
	UINT32 Mode;
	///
	/// Pointer to read-only EFI_GRAPHICS_OUTPUT_MODE_INFORMATION data.
	///
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
	///
	/// Size of Info structure in bytes.
	///
	UINTN SizeOfInfo;
	///
	/// Base address of graphics linear frame buffer.
	/// Offset zero in FrameBufferBase represents the upper left pixel of the display.
	///
	EFI_PHYSICAL_ADDRESS FrameBufferBase;
	///
	/// Amount of frame buffer needed to support the active mode as defined by
	/// PixelsPerScanLine x VerticalResolution x PixelElementSize.
	///
	UINTN FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

///
/// Provides a basic abstraction to set video modes and copy pixels to and from
/// the graphics controller's frame buffer. The linear address of the hardware
/// frame buffer is also exposed so software can write directly to the video hardware.
///
struct _EFI_GRAPHICS_OUTPUT_PROTOCOL {
	EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE QueryMode;
	EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE   SetMode;
	VOID* Blt;
	///
	/// Pointer to EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE data.
	///
	EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *Mode;
};

extern EFI_GUID gEfiGraphicsOutputProtocolGuid;

