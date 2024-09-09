#pragma once

#include "Uefi.h"

/// Reads the whole file into memory and returns its memory address via the fileBuffer parameter.
EFI_STATUS ReadFile(EFI_SYSTEM_TABLE* systemTable, EFI_FILE_PROTOCOL* rootVolume, CHAR16* fileName, VOID** fileBuffer);

