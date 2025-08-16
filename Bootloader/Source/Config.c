#include "Config.h"

#include "Memory.h"

/// Advances `filePointer` to the beginning of the next line. Returns `false` when the end of the file has been reached, `true` otherwise.
///
/// Only handles Linux-style newlines, `fileEnd` is exclusive.
static VOID SkipToNextLine(const INT8** filePointer, const INT8* fileEnd)
{
	while (*filePointer < fileEnd && **filePointer != '\n') {
		++(*filePointer);
	}

	if (*filePointer < fileEnd) {
		++(*filePointer);
	}
}

static VOID SkipWhitespace(const INT8** filePointer, const INT8* fileEnd)
{
	while (*filePointer < fileEnd && (**filePointer == ' ' || **filePointer == '\t' || **filePointer == '\n')) {
		++(*filePointer);
	}
}

static UINTN ConfigKeyLength(const INT8* keyBegin, const INT8* fileEnd)
{
	UINTN length = 0;

	while (keyBegin < fileEnd && *keyBegin != ' ' && *keyBegin != '\t' && *keyBegin != '=' && *keyBegin != '\n') {
		++length;
		++keyBegin;
	}

	return length;
}

static UINTN ConfigValueLength(const INT8* valueBegin, const INT8* fileEnd)
{
	UINTN length = 0;

	while (valueBegin < fileEnd && *valueBegin != '\n') {
		++length;
		++valueBegin;
	}

	return length;
}

EFI_STATUS GetConfigValue(const INT8* configFile, UINTN configFileSize, const INT8* key, const INT8** value, UINTN* valueLength)
{
	const INT8* fileEnd = configFile + configFileSize;

	while (configFile < fileEnd) {
		SkipWhitespace(&configFile, fileEnd);

		UINTN keyLength = ConfigKeyLength(configFile, fileEnd);

		if (MemoryCompare(configFile, key, keyLength) != 0) {
			continue;
		}

		configFile += keyLength;

		SkipWhitespace(&configFile, fileEnd);

		if (*configFile == '=') {
			++configFile;
			SkipWhitespace(&configFile, fileEnd);

			*value = configFile;
			*valueLength = ConfigValueLength(*value, fileEnd);
			return EFI_SUCCESS;
		}

		SkipToNextLine(&configFile, fileEnd);
	}

	return EFI_NOT_FOUND;
}
