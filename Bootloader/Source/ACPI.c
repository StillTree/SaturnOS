#include "ACPI.h"

#include "Logger.h"

EFI_STATUS FindXSDT(EFI_SYSTEM_TABLE* systemTable, VOID** xsdpPointer)
{
	for (UINTN i = 0; i < systemTable->NumberOfTableEntries; i++) {
		if (CompareGuid(&systemTable->ConfigurationTable[i].VendorGuid, &gEfiAcpi20TableGuid)) {
			XSDP* xsdp = systemTable->ConfigurationTable[i].VendorTable;

			// The two checksums are just the XSDP's standard and extended parts' individual bytes added up,
			// the XSDP is valid if both of its checksums' lower byte will be equal to 0.
			UINTN sum10 = 0;
			UINTN sum20 = 0;

			for(UINTN i = 0; i < 20; i++) {
				sum10 += *((UINT8*)xsdp + i);
			}

			for(UINTN i = 20; i < 33; i++) {
				sum20 += *((UINT8*)xsdp + i);
			}

			// Validate the revision number and the XSDP's computed checksum
			if(xsdp->Revision == 2 && (sum10 & 0xff) == 0 && (sum20 & 0xff) == 0) {
				SN_LOG_INFO(L"Successfully found the ACPI 2.0 XSDP");
				*xsdpPointer = xsdp;
				return EFI_SUCCESS;
			}
		}
	}

	SN_LOG_ERROR(L"Could not find the ACPI 2.0 XSDP");

	return EFI_NOT_FOUND;
}

BOOLEAN CompareGuid(const EFI_GUID* Guid1, const EFI_GUID* Guid2)
{
	UINT64 LowPartOfGuid1;
	UINT64 LowPartOfGuid2;
	UINT64 HighPartOfGuid1;
	UINT64 HighPartOfGuid2;

	LowPartOfGuid1 = *((const UINT64*)Guid1);
	LowPartOfGuid2 = *((const UINT64*)Guid2);
	HighPartOfGuid1 = *((const UINT64*)Guid1 + 1);
	HighPartOfGuid2 = *((const UINT64*)Guid2 + 1);

	return (BOOLEAN)(LowPartOfGuid1 == LowPartOfGuid2 && HighPartOfGuid1 == HighPartOfGuid2);
}
