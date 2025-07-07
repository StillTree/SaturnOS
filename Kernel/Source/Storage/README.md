# SaturnOS's storage architecture

The bootloader loads the initial ramdisk which, after some initialisation, gets parsed by the kernel.
It contains some essential drivers for things like: filesystems, storage devices and etc.

There is planned a memory-only mode, where SaturnOS pulls everything just from the ramdisk
and won't interact with any mass storage devices.

## Ramdisk

The ramdisk is populated with some essential drivers that are then used to further bootstrap the system.
It's loaded into memory thanks to the bootloader which then passes all the necessary info to the kernel.
All of the files are stored in a "made up" filesystem created just for the purpose of being used in ramdisks - STFS.

## STFS - SaturnOS File System

A comically simple filesystem used in the ramdisk. It consists of three essential elements:
- Superblock (contains all the essential information)
- File list (a flat list of all the files the ramdisk contains)
- File contents (essentially the files themselves)

### The superblock

Memory layout starting from the beginning of the ramdisk:

| Offset (B) | Size (B) | Field Name | Description                          |
|------------|----------|------------|--------------------------------------|
| `0x00`     | 4        | Signature  | Always "STFS" - `0x53465453`         |
| `0x04`     | 4        | Zeroed     | Always zeroes                        |
| `0x08`     | 8        | File count | Total number of files in the ramdisk |

### File list

It starts right after the superblock and contains a list of files on the ramdisk.
Layout of each entry:

| Offset (B) | Size (B) | Field Name     | Description                                               |
|------------|----------|----------------|-----------------------------------------------------------|
| `0x00`     | 32       | File name      | The name of the file, null-padded.                        |
| `0x20`     | 8        | File size      | The size of the file's contents.                          |
| `0x28`     | 8        | Content offset | Offset of the file's contents from the ramdisk beginning. |
| `0x30`     | 8        | File ID        | A unique identifier for this file.                        |

## VFS

When opening a file, a handle (1-based pointer to the process's file descriptor array) is returned,
every process has its own file descriptor array that consists of a pointer to the actual kernel's opened file structure
and in the future will most likely contain some more process exclusive information.
This is done so a process can't just simply open another process's file.

SaturnOS keeps an array of all the opened files which point directly to the mountpoint they're associated with.
These are shared between processes, are reference counted and are pointed to by each processes' file descriptors.
When a process closes a file, the kernel makes sure that there are no remaining ghost opened files
(opened files with no references).

A file can be written to only by a single process at a time, while being read by any number of processes.

A mountpoint contains some crucial information about itself but most importantly pointers to functions that write and read from a file.
This is a temporary solution until I can create a sane IPC system to move all of the file handling
(apart from the ramdisk of course) out of the core kernel and into some external drivers.
