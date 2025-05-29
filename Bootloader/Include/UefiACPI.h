#pragma once

#include "UefiTypes.h"

#define EFI_ACPI_20_TABLE_GUID { 0x8868e871, 0xe4f1, 0x11d3, { 0xbc, 0x22, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 } }

///
/// Contains a set of GUID/pointer pairs comprised of the ConfigurationTable field in the
/// EFI System Table.
///
typedef struct {
	///
	/// The 128-bit GUID value that uniquely identifies the system configuration table.
	///
	EFI_GUID VendorGuid;
	///
	/// A pointer to the table associated with VendorGuid.
	///
	VOID* VendorTable;
} EFI_CONFIGURATION_TABLE;

extern EFI_GUID gEfiAcpi20TableGuid;
