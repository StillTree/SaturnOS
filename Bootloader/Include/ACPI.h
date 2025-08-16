#pragma once

#include "Uefi.h"

typedef struct __attribute__((packed)) XSDP {
	INT8 Signature[8];
	UINT8 Checksum;
	INT8 OEMID[6];
	UINT8 Revision;
	UINT32 RsdtAddress;

	UINT32 Length;
	UINT64 XsdtAddress;
	UINT8 ExtendedChecksum;
	UINT8 reserved[3];
} XSDP;

EFI_STATUS FindXSDP(EFI_SYSTEM_TABLE* systemTable, VOID** xsdpPointer);
BOOLEAN CompareGuid(const EFI_GUID* Guid1, const EFI_GUID* Guid2);
