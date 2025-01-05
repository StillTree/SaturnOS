#pragma once

#include "UefiTypes.h"

#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID { 0x964e5b22, 0x6459, 0x11d2, { 0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b } }

#define EFI_FILE_INFO_ID { 0x09576e92, 0x6d3f, 0x11d2, { 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b } }

typedef struct {
	UINT64 Size;
	UINT64 FileSize;
	UINT64 PhysicalSize;
	EFI_TIME CreateTime;
	EFI_TIME LastAccessTime;
	EFI_TIME ModificationTime;
	UINT64 Attribute;
	CHAR16 FileName[];
} EFI_FILE_INFO;

typedef struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

typedef struct _EFI_FILE_PROTOCOL EFI_FILE_PROTOCOL;

/**
	Open the root directory on a volume.

	@param  This A pointer to the volume to open the root directory.
	@param  Root A pointer to the location to return the opened file handle for the
				 root directory.

	@retval EFI_SUCCESS          The device was opened.
	@retval EFI_UNSUPPORTED      This volume does not support the requested file system type.
	@retval EFI_NO_MEDIA         The device has no medium.
	@retval EFI_DEVICE_ERROR     The device reported an error.
	@retval EFI_VOLUME_CORRUPTED The file system structures are corrupted.
	@retval EFI_ACCESS_DENIED    The service denied access to the file.
	@retval EFI_OUT_OF_RESOURCES The volume was not opened due to lack of resources.
	@retval EFI_MEDIA_CHANGED    The device has a different medium in it or the medium is no
								 longer supported. Any existing file handles for this volume are
								 no longer valid. To access the files on the new medium, the
								 volume must be reopened with OpenVolume().

**/
typedef EFI_STATUS(EFIAPI* EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME)(
	IN EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* This, OUT EFI_FILE_PROTOCOL** Root);

struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL {
	///
	/// The version of the EFI_SIMPLE_FILE_SYSTEM_PROTOCOL. The version
	/// specified by this specification is 0x00010000. All future revisions
	/// must be backwards compatible.
	///
	UINT64 Revision;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME OpenVolume;
};

/**
	Opens a new file relative to the source file's location.

	@param  This       A pointer to the EFI_FILE_PROTOCOL instance that is the file
					   handle to the source location. This would typically be an open
					   handle to a directory.
	@param  NewHandle  A pointer to the location to return the opened handle for the new
					   file.
	@param  FileName   The Null-terminated string of the name of the file to be opened.
					   The file name may contain the following path modifiers: "\", ".",
					   and "..".
	@param  OpenMode   The mode to open the file. The only valid combinations that the
					   file may be opened with are: Read, Read/Write, or Create/Read/Write.
	@param  Attributes Only valid for EFI_FILE_MODE_CREATE, in which case these are the
					   attribute bits for the newly created file.

	@retval EFI_SUCCESS          The file was opened.
	@retval EFI_NOT_FOUND        The specified file could not be found on the device.
	@retval EFI_NO_MEDIA         The device has no medium.
	@retval EFI_MEDIA_CHANGED    The device has a different medium in it or the medium is no
								 longer supported.
	@retval EFI_DEVICE_ERROR     The device reported an error.
	@retval EFI_VOLUME_CORRUPTED The file system structures are corrupted.
	@retval EFI_WRITE_PROTECTED  An attempt was made to create a file, or open a file for write
								 when the media is write-protected.
	@retval EFI_ACCESS_DENIED    The service denied access to the file.
	@retval EFI_OUT_OF_RESOURCES Not enough resources were available to open the file.
	@retval EFI_VOLUME_FULL      The volume is full.

**/
typedef EFI_STATUS(EFIAPI* EFI_FILE_OPEN)(
	IN EFI_FILE_PROTOCOL* This, OUT EFI_FILE_PROTOCOL** NewHandle, IN CHAR16* FileName, IN UINT64 OpenMode, IN UINT64 Attributes);

//
// Open modes
//
#define EFI_FILE_MODE_READ 0x0000000000000001ULL
#define EFI_FILE_MODE_WRITE 0x0000000000000002ULL
#define EFI_FILE_MODE_CREATE 0x8000000000000000ULL

//
// File attributes
//
#define EFI_FILE_READ_ONLY 0x0000000000000001ULL
#define EFI_FILE_HIDDEN 0x0000000000000002ULL
#define EFI_FILE_SYSTEM 0x0000000000000004ULL
#define EFI_FILE_RESERVED 0x0000000000000008ULL
#define EFI_FILE_DIRECTORY 0x0000000000000010ULL
#define EFI_FILE_ARCHIVE 0x0000000000000020ULL
#define EFI_FILE_VALID_ATTR 0x0000000000000037ULL

/**
	Closes a specified file handle.

	@param  This          A pointer to the EFI_FILE_PROTOCOL instance that is the file
						  handle to close.

	@retval EFI_SUCCESS   The file was closed.

**/
typedef EFI_STATUS(EFIAPI* EFI_FILE_CLOSE)(IN EFI_FILE_PROTOCOL* This);

/**
	Reads data from a file.

	@param  This       A pointer to the EFI_FILE_PROTOCOL instance that is the file
					   handle to read data from.
	@param  BufferSize On input, the size of the Buffer. On output, the amount of data
					   returned in Buffer. In both cases, the size is measured in bytes.
	@param  Buffer     The buffer into which the data is read.

	@retval EFI_SUCCESS          Data was read.
	@retval EFI_NO_MEDIA         The device has no medium.
	@retval EFI_DEVICE_ERROR     The device reported an error.
	@retval EFI_DEVICE_ERROR     An attempt was made to read from a deleted file.
	@retval EFI_DEVICE_ERROR     On entry, the current file position is beyond the end of the file.
	@retval EFI_VOLUME_CORRUPTED The file system structures are corrupted.
	@retval EFI_BUFFER_TOO_SMALL The BufferSize is too small to read the current directory
								 entry. BufferSize has been updated with the size
								 needed to complete the request.

**/
typedef EFI_STATUS(EFIAPI* EFI_FILE_READ)(IN EFI_FILE_PROTOCOL* This, IN OUT UINTN* BufferSize, OUT VOID* Buffer);

/**
	Returns information about a file.

	@param  This            A pointer to the EFI_FILE_PROTOCOL instance that is the file
							handle the requested information is for.
	@param  InformationType The type identifier for the information being requested.
	@param  BufferSize      On input, the size of Buffer. On output, the amount of data
							returned in Buffer. In both cases, the size is measured in bytes.
	@param  Buffer          A pointer to the data buffer to return. The buffer's type is
							indicated by InformationType.

	@retval EFI_SUCCESS          The information was returned.
	@retval EFI_UNSUPPORTED      The InformationType is not known.
	@retval EFI_NO_MEDIA         The device has no medium.
	@retval EFI_DEVICE_ERROR     The device reported an error.
	@retval EFI_VOLUME_CORRUPTED The file system structures are corrupted.
	@retval EFI_BUFFER_TOO_SMALL The BufferSize is too small to read the current directory entry.
								 BufferSize has been updated with the size needed to complete
								 the request.
**/
typedef EFI_STATUS(EFIAPI* EFI_FILE_GET_INFO)(
	IN EFI_FILE_PROTOCOL* This, IN EFI_GUID* InformationType, IN OUT UINTN* BufferSize, OUT VOID* Buffer);

///
/// The EFI_FILE_PROTOCOL provides file IO access to supported file systems.
/// An EFI_FILE_PROTOCOL provides access to a file's or directory's contents,
/// and is also a reference to a location in the directory tree of the file system
/// in which the file resides. With any given file handle, other files may be opened
/// relative to this file's location, yielding new file handles.
///
struct _EFI_FILE_PROTOCOL {
	///
	/// The version of the EFI_FILE_PROTOCOL interface. The version specified
	/// by this specification is EFI_FILE_PROTOCOL_LATEST_REVISION.
	/// Future versions are required to be backward compatible to version 1.0.
	///
	UINT64 Revision;
	EFI_FILE_OPEN Open;
	EFI_FILE_CLOSE Close;
	VOID* Delete;
	EFI_FILE_READ Read;
	VOID* Write;
	VOID* GetPosition;
	VOID* SetPosition;
	EFI_FILE_GET_INFO GetInfo;
	VOID* SetInfo;
	VOID* Flush;
	VOID* OpenEx;
	VOID* ReadEx;
	VOID* WriteEx;
	VOID* FlushEx;
};

extern EFI_GUID gEfiSimpleFileSystemProtocolGuid;
