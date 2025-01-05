#pragma once

#include "Uefi.h"

#define EFI_LOADED_IMAGE_PROTOCOL_GUID { 0x5B1B31A1, 0x9562, 0x11d2, { 0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B } }

///
/// Can be used on any image handle to obtain information about the loaded image.
///
typedef struct {
	UINT32 Revision; ///< Defines the revision of the EFI_LOADED_IMAGE_PROTOCOL structure.
					 ///< All future revisions will be backward compatible to the current revision.
	EFI_HANDLE ParentHandle; ///< Parent image's image handle. NULL if the image is loaded directly from
							 ///< the firmware's boot manager.
	VOID* SystemTable; ///< the image's EFI system table pointer.

	//
	// Source location of image
	//
	EFI_HANDLE DeviceHandle; ///< The device handle that the EFI Image was loaded from.
	VOID* FilePath; ///< A pointer to the file path portion specific to DeviceHandle
					///< that the EFI Image was loaded from.
	VOID* Reserved; ///< Reserved. DO NOT USE.

	//
	// Images load options
	//
	UINT32 LoadOptionsSize; ///< The size in bytes of LoadOptions.
	VOID* LoadOptions; ///< A pointer to the image's binary load options.

	//
	// Location of where image was loaded
	//
	VOID* ImageBase; ///< The base address at which the image was loaded.
	UINT64 ImageSize; ///< The size in bytes of the loaded image.
	EFI_MEMORY_TYPE ImageCodeType; ///< The memory type that the code sections were loaded as.
	EFI_MEMORY_TYPE ImageDataType; ///< The memory type that the data sections were loaded as.
	VOID* Unload;
} EFI_LOADED_IMAGE_PROTOCOL;

extern EFI_GUID gEfiLoadedImageProtocolGuid;
