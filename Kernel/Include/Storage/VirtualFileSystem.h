#pragma once

#include "Core.h"
#include "Memory/SizedBlockAllocator.h"
#include "Result.h"

constexpr usz MAX_MOUNTPOINTS = 32;
constexpr usz MAX_OPENED_FILES = 64;

typedef struct VirtualFileSystem {
	SizedBlockAllocator Mountpoints;
	SizedBlockAllocator OpenedFiles;
	/// A bitmap of used letters, where bit 0 is A and bit 25 is Z.
	u32 UsedMountLetters;
} VirtualFileSystem;

typedef enum MountpointCapabilities { MountpointReadable = 1, MountpointWriteable = 2 } MountpointCapabilities;

typedef struct MountpointFunctions {
	void (*ReadFile)(const i8* relativeFileName, void* buffer);
} MountpointFunctions;

typedef struct Mountpoint {
	i8 MountLetter;
	MountpointCapabilities Capabilities;

	MountpointFunctions Functions;
} Mountpoint;

typedef struct OpenedFile {
	Mountpoint* Mountpoint;
	usz References;
} OpenedFile;

typedef struct ProcessFileDescriptor {
	OpenedFile* OpenedFile;
	usz OffsetBytes;
} ProcessFileDescriptor;

Result InitVirtualFileSystem(VirtualFileSystem* fileSystem);
Result GetFirstUnusedMountLetter(VirtualFileSystem* fileSystem, i8* mountLetter);
Result SetMountLetterStatus(VirtualFileSystem* fileSystem, i8 mountLetter, bool reserved);
/// Returns `true` for reserved mount letters and `false` for unreserved ones.
bool GetMountLetterStatus(VirtualFileSystem* fileSystem, i8 mountLetter);

/// If `mountLetter` is 0, the first available letter will be chosen automatically.
Result CreateMountpoint(VirtualFileSystem* fileSystem, i8 mountLetter, MountpointCapabilities capabilities, MountpointFunctions functions);
Result DeleteMountpoint(VirtualFileSystem* fileSystem, i8 mountLetter);

extern VirtualFileSystem g_virtualFileSystem;
