#pragma once

#include "UefiTypes.h"
#include "UefiBootServices.h"
#include "UefiRuntimeServices.h"
#include "UefiProtocols/SimpleTextOutput.h"
#include "UefiProtocols/GraphicsOutput.h"

///
/// EFI System Table
///
typedef struct {
	///
	/// The table header for the EFI System Table.
	///
	EFI_TABLE_HEADER Hdr;
	///
	/// A pointer to a null terminated string that identifies the vendor
	/// that produces the system firmware for the platform.
	///
	CHAR16* FirmwareVendor;
	///
	/// A firmware vendor specific value that identifies the revision
	/// of the system firmware for the platform.
	///
	UINT32 FirmwareRevision;
	///
	/// The handle for the active console input device. This handle must support
	/// EFI_SIMPLE_TEXT_INPUT_PROTOCOL and EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL. If
	/// there is no active console, these protocols must still be present.
	///
	EFI_HANDLE ConsoleInHandle;
	///
	/// A pointer to the EFI_SIMPLE_TEXT_INPUT_PROTOCOL interface that is
	/// associated with ConsoleInHandle.
	///
	VOID* ConIn;
	///
	/// The handle for the active console output device. This handle must support the
	/// EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL. If there is no active console, these protocols
	/// must still be present.
	///
	EFI_HANDLE ConsoleOutHandle;
	///
	/// A pointer to the EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL interface
	/// that is associated with ConsoleOutHandle.
	///
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* ConOut;
	///
	/// The handle for the active standard error console device.
	/// This handle must support the EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL. If there
	/// is no active console, this protocol must still be present.
	///
	EFI_HANDLE StandardErrorHandle;
	///
	/// A pointer to the EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL interface
	/// that is associated with StandardErrorHandle.
	///
	VOID* StdErr;
	///
	/// A pointer to the EFI Runtime Services Table.
	///
	EFI_RUNTIME_SERVICES* RuntimeServices;
	///
	/// A pointer to the EFI Boot Services Table.
	///
	EFI_BOOT_SERVICES* BootServices;
	///
	/// The number of system configuration tables in the buffer ConfigurationTable.
	///
	UINTN NumberOfTableEntries;
	///
	/// A pointer to the system configuration tables.
	/// The number of entries in the table is NumberOfTableEntries.
	///
	VOID* ConfigurationTable;
} EFI_SYSTEM_TABLE;

