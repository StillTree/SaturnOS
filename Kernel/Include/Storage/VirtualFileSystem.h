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

typedef struct OpenedFileInformation {
	usz Size;
} OpenedFileInformation;

/// These mode numbers correspond to those of `OpenedFileMode`.
typedef enum MountpointCapabilities : u8 { MountpointReadable = 1, MountpointWriteable = 2 } MountpointCapabilities;

typedef struct MountpointFunctions {
	Result (*FileOpen)(const i8* relativeFileName, void** fileSystemSpecific);
	Result (*FileRead)(void* fileSystemSpecific, usz fileOffset, usz countBytes, void* buffer);
	Result (*FileInformation)(void* fileSystemSpecific, OpenedFileInformation* fileInformation);
	Result (*FileClose)(void* fileSystemSpecific);
} MountpointFunctions;

typedef struct Mountpoint {
	i8 MountLetter;
	MountpointCapabilities Capabilities;

	MountpointFunctions Functions;
} Mountpoint;

typedef struct OpenedFile {
	Mountpoint* Mountpoint;
	usz References;
	OpenedFileInformation CachedInformation;
	void* FileSystemSpecific;
} OpenedFile;

typedef struct ProcessFileDescriptor {
	OpenedFile* OpenedFile;
	usz OffsetBytes;
} ProcessFileDescriptor;

/// These mode numbers correspond to those of `MountpointCapabilities`.
typedef enum OpenedFileMode : u8 { OpenFileRead = 1, OpenFileWrite = 2, OpenFileExclusive = 3 } OpenedFileMode;

Result InitVirtualFileSystem(VirtualFileSystem* fileSystem);
Result GetFirstUnusedMountLetter(VirtualFileSystem* fileSystem, i8* mountLetter);
Result SetMountLetterStatus(VirtualFileSystem* fileSystem, i8 mountLetter, bool reserved);
/// Returns `true` for reserved mount letters and `false` for unreserved ones.
bool GetMountLetterStatus(VirtualFileSystem* fileSystem, i8 mountLetter);

/// If `mountLetter` is 0, the first available letter will be chosen automatically.
Result MountpointCreate(VirtualFileSystem* fileSystem, i8 mountLetter, MountpointCapabilities capabilities, MountpointFunctions functions);
Result MountpointDelete(VirtualFileSystem* fileSystem, i8 mountLetter);
Result MountpointGetFromLetter(VirtualFileSystem* fileSystem, i8 mountLetter, Mountpoint** mountpoint);

/// This functions should be called only when the interrupt flag is cleared. It can be set afterwards.
Result FileOpen(VirtualFileSystem* fileSystem, const i8* path, OpenedFileMode mode, ProcessFileDescriptor** fileDescriptor);
Result FileRead(ProcessFileDescriptor* fileDescriptor, usz countBytes, void* buffer);
Result FileInformation(ProcessFileDescriptor* fileDescriptor, OpenedFileInformation* fileInformation);
Result FileClose(VirtualFileSystem* fileSystem, ProcessFileDescriptor* fileDescriptor);

extern VirtualFileSystem g_virtualFileSystem;
