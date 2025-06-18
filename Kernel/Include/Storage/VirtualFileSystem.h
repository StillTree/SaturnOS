#pragma once

#include "Core.h"
#include "Memory/SizedBlockAllocator.h"
#include "Result.h"

constexpr usz MAX_MOUNTPOINTS = 26; // The number of latin characters

typedef struct VirtualFileSystem {
	SizedBlockAllocator Mountpoints;
	/// A bitmap of used letters, where bit 0 is A and bit 25 is Z.
	u32 UsedMountLetters;
} VirtualFileSystem;

typedef enum VFSMountCapabilities {
	VFSMountReadable = 1,
	VFSMountWriteable = 2
} VFSMountCapabilities;

typedef struct Mountpoint {
	i8 MountLetter;
	void* Driver;
	VFSMountCapabilities Capabilities;
} Mountpoint;

Result InitVirtualFileSystem(VirtualFileSystem* fileSystem);
Result GetFirstUnusedMountLetter(VirtualFileSystem* fileSystem, i8* mountLetter);
Result SetMountLetterStatus(VirtualFileSystem* fileSystem, i8 mountLetter, bool reserved);
/// Returns `true` for reserved mount letters and `false` for unreserved ones.
bool GetMountLetterStatus(VirtualFileSystem* fileSystem, i8 mountLetter);

extern VirtualFileSystem g_virtualFileSystem;
