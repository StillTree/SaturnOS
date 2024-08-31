#pragma once

typedef unsigned long long UINT64;
typedef unsigned int       UINT32;
typedef unsigned short     CHAR16;
typedef unsigned short     UINT16;
typedef unsigned char      UINT8;

typedef UINT64 UINTN;

typedef unsigned char BOOLEAN;

#define EFIAPI __attribute__((ms_abi))

///
/// Datum is passed to the function.
///
#define IN

///
/// Datum is returned from the function.
///
#define OUT

///
/// Passing the datum to the function is optional, and a NULL
/// is passed if the value is not supplied.
///
#define OPTIONAL

///
/// Undeclared type.
///
#define VOID void

//
// Status codes common to all execution phases
//
typedef UINTN EFI_STATUS;

///
/// A collection of related interfaces.
///
typedef VOID* EFI_HANDLE;

///
/// 64-bit physical memory address.
///
typedef UINT64 EFI_PHYSICAL_ADDRESS;

///
/// 64-bit virtual memory address.
///
typedef UINT64 EFI_VIRTUAL_ADDRESS;

///
/// Boolean true value.  UEFI Specification defines this value to be 1,
/// but this form is more portable.
///
#define TRUE ((BOOLEAN) (1==1))

///
/// Boolean false value.  UEFI Specification defines this value to be 0,
/// but this form is more portable.
///
#define FALSE ((BOOLEAN) (0==1))

///
/// The operation completed successfully.
///
#define EFI_SUCCESS (EFI_STATUS) (0)

///
/// 128 bit buffer containing a unique identifier value.
/// Unless otherwise specified, aligned on a 64 bit boundary.
///
typedef struct {
	UINT32 Data1;
	UINT16 Data2;
	UINT16 Data3;
	UINT8  Data4[8];
} EFI_GUID;

///
/// Data structure that precedes all of the standard EFI table types.
///
typedef struct {
	///
	/// A 64-bit signature that identifies the type of table that follows.
	/// Unique signatures have been generated for the EFI System Table,
	/// the EFI Boot Services Table, and the EFI Runtime Services Table.
	///
	UINT64 Signature;
	///
	/// The revision of the EFI Specification to which this table
	/// conforms. The upper 16 bits of this field contain the major
	/// revision value, and the lower 16 bits contain the minor revision
	/// value. The minor revision values are limited to the range of 00..99.
	///
	UINT32 Revision;
	///
	/// The size, in bytes, of the entire table including the EFI_TABLE_HEADER.
	///
	UINT32 HeaderSize;
	///
	/// The 32-bit CRC for the entire table. This value is computed by
	/// setting this field to 0, and computing the 32-bit CRC for HeaderSize bytes.
	///
	UINT32 CRC32;
	///
	/// Reserved field that must be set to 0.
	///
	UINT32 Reserved;
} EFI_TABLE_HEADER;

