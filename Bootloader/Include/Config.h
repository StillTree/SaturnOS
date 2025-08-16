#pragma once

#include "Uefi.h"

/// Linearly parses the given config file and returns the corresponding value.
///
/// Note: Normally, UEFI uses 2-byte wide strings (UCS-2/UTF-16) for file paths and text.
/// However, since this configuration file is currently only used for passing kernel arguments,
/// and the kernel only understands standard ASCII (or UTF-8),
/// the file contents will be handled as simple ASCII strings for simplicity.
EFI_STATUS GetConfigValue(const INT8* configFile, UINTN configFileSize, const INT8* key, const INT8** value, UINTN* valueLength);
