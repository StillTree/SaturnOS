#pragma once

typedef unsigned long long UINT64;
typedef long long INT64;
typedef unsigned int UINT32;
typedef int INT32;
typedef unsigned short CHAR16;
typedef unsigned short UINT16;
typedef short INT16;
typedef unsigned char UINT8;
typedef char INT8;

typedef UINT64 UINTN;
typedef INT64 INTN;

#define MAX_BIT 0x8000000000000000ULL

typedef unsigned char BOOLEAN;

///
/// Boolean true value.  UEFI Specification defines this value to be 1,
/// but this form is more portable.
///
#define TRUE ((BOOLEAN)(1 == 1))

///
/// Boolean false value.  UEFI Specification defines this value to be 0,
/// but this form is more portable.
///
#define FALSE ((BOOLEAN)(0 == 1))

///
/// NULL pointer (VOID *)
///
#define NULL ((VOID*)0)

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

/**
	Produces a RETURN_STATUS code with the highest bit set.

	@param  StatusCode    The status code value to convert into a warning code.
						  StatusCode must be in the range 0x00000000..0x7FFFFFFF.

	@return The value specified by StatusCode with the highest bit set.

**/
#define ENCODE_ERROR(StatusCode) ((EFI_STATUS)(MAX_BIT | (StatusCode)))

/**
	Returns TRUE if a specified EFI_STATUS code is an error code.

	This function returns TRUE if StatusCode has the high bit set.  Otherwise, FALSE is returned.

	@param  StatusCode    The status code value to evaluate.

	@retval TRUE          The high bit of StatusCode is set.
	@retval FALSE         The high bit of StatusCode is clear.

**/
#define EFI_ERROR(StatusCode) (((INTN)(EFI_STATUS)(StatusCode)) < 0)

///
/// The operation completed successfully.
///
#define EFI_SUCCESS (EFI_STATUS)(0)

///
/// The parameter was incorrect.
///
#define EFI_INVALID_PARAMETER ENCODE_ERROR(2)

///
/// The operation is not supported.
///
#define EFI_UNSUPPORTED ENCODE_ERROR(3)

///
/// The buffer was not large enough to hold the requested data.
/// The required buffer size is returned in the appropriate
/// parameter when this error occurs.
///
#define EFI_BUFFER_TOO_SMALL ENCODE_ERROR(5)

///
/// The physical device reported an error while attempting the
/// operation.
///
#define EFI_DEVICE_ERROR ENCODE_ERROR(7)

///
/// The resource has run out.
///
#define EFI_OUT_OF_RESOURCES ENCODE_ERROR(9)

///
/// The item was not found.
///
#define EFI_NOT_FOUND ENCODE_ERROR(14)

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
/// 128 bit buffer containing a unique identifier value.
/// Unless otherwise specified, aligned on a 64 bit boundary.
///
typedef struct {
	UINT32 Data1;
	UINT16 Data2;
	UINT16 Data3;
	UINT8 Data4[8];
} EFI_GUID;

///
/// EFI Time Abstraction:
///  Year:       1900 - 9999
///  Month:      1 - 12
///  Day:        1 - 31
///  Hour:       0 - 23
///  Minute:     0 - 59
///  Second:     0 - 59
///  Nanosecond: 0 - 999,999,999
///  TimeZone:   -1440 to 1440 or 2047
///
typedef struct {
	UINT16 Year;
	UINT8 Month;
	UINT8 Day;
	UINT8 Hour;
	UINT8 Minute;
	UINT8 Second;
	UINT8 Pad1;
	UINT32 Nanosecond;
	INT16 TimeZone;
	UINT8 Daylight;
	UINT8 Pad2;
} EFI_TIME;

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
