#pragma once

#include "Uefi.h"

EFI_STATUS ReadFile(EFI_SYSTEM_TABLE* systemTable, EFI_FILE_PROTOCOL* rootVolume, CHAR16* fileName, VOID** fileBuffer);

