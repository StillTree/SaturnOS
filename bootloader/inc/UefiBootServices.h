#pragma once

#include "UefiTypes.h"

///
/// Definition of an EFI memory descriptor.
///
typedef struct {
	///
	/// Type of the memory region.
	/// Type EFI_MEMORY_TYPE is defined in the
	/// AllocatePages() function description.
	///
	UINT32 Type;
	///
	/// Physical address of the first byte in the memory region. PhysicalStart must be
	/// aligned on a 4 KiB boundary, and must not be above 0xfffffffffffff000. Type
	/// EFI_PHYSICAL_ADDRESS is defined in the AllocatePages() function description
	///
	EFI_PHYSICAL_ADDRESS PhysicalStart;
	///
	/// Virtual address of the first byte in the memory region.
	/// VirtualStart must be aligned on a 4 KiB boundary,
	/// and must not be above 0xfffffffffffff000.
	///
	EFI_VIRTUAL_ADDRESS VirtualStart;
	///
	/// NumberOfPagesNumber of 4 KiB pages in the memory region.
	/// NumberOfPages must not be 0, and must not be any value
	/// that would represent a memory page with a start address,
	/// either physical or virtual, above 0xfffffffffffff000.
	///
	UINT64 NumberOfPages;
	///
	/// Attributes of the memory region that describe the bit mask of capabilities
	/// for that memory region, and not necessarily the current settings for that
	/// memory region.
	///
	UINT64 Attribute;
} EFI_MEMORY_DESCRIPTOR;

///
/// Enumeration of memory types introduced in UEFI.
///
typedef enum {
	///
	/// Not used.
	///
	EfiReservedMemoryType,
	///
	/// The code portions of a loaded application.
	/// (Note that UEFI OS loaders are UEFI applications.)
	///
	EfiLoaderCode,
	///
	/// The data portions of a loaded application and the default data allocation
	/// type used by an application to allocate pool memory.
	///
	EfiLoaderData,
	///
	/// The code portions of a loaded Boot Services Driver.
	///
	EfiBootServicesCode,
	///
	/// The data portions of a loaded Boot Serves Driver, and the default data
	/// allocation type used by a Boot Services Driver to allocate pool memory.
	///
	EfiBootServicesData,
	///
	/// The code portions of a loaded Runtime Services Driver.
	///
	EfiRuntimeServicesCode,
	///
	/// The data portions of a loaded Runtime Services Driver and the default
	/// data allocation type used by a Runtime Services Driver to allocate pool memory.
	///
	EfiRuntimeServicesData,
	///
	/// Free (unallocated) memory.
	///
	EfiConventionalMemory,
	///
	/// Memory in which errors have been detected.
	///
	EfiUnusableMemory,
	///
	/// Memory that holds the ACPI tables.
	///
	EfiACPIReclaimMemory,
	///
	/// Address space reserved for use by the firmware.
	///
	EfiACPIMemoryNVS,
	///
	/// Used by system firmware to request that a memory-mapped IO region
	/// be mapped by the OS to a virtual address so it can be accessed by EFI runtime services.
	///
	EfiMemoryMappedIO,
	///
	/// System memory-mapped IO region that is used to translate memory
	/// cycles to IO cycles by the processor.
	///
	EfiMemoryMappedIOPortSpace,
	///
	/// Address space reserved by the firmware for code that is part of the processor.
	///
	EfiPalCode,
	///
	/// A memory region that operates as EfiConventionalMemory,
	/// however it happens to also support byte-addressable non-volatility.
	///
	EfiPersistentMemory,
	///
	/// A memory region that describes system memory that has not been accepted
	/// by a corresponding call to the underlying isolation architecture.
	///
	EfiUnacceptedMemoryType,
	EfiMaxMemoryType
} EFI_MEMORY_TYPE;

/**
	Returns the current memory map.

	@param[in, out]  MemoryMapSize         A pointer to the size, in bytes, of the MemoryMap buffer.
	                                       On input, this is the size of the buffer allocated by the caller.
	                                       On output, it is the size of the buffer returned by the firmware if
	                                       the buffer was large enough, or the size of the buffer needed to contain
	                                       the map if the buffer was too small.
	@param[out]      MemoryMap             A pointer to the buffer in which firmware places the current memory
	                                       map.
	@param[out]      MapKey                A pointer to the location in which firmware returns the key for the
	                                       current memory map.
	@param[out]      DescriptorSize        A pointer to the location in which firmware returns the size, in bytes, of
	                                       an individual EFI_MEMORY_DESCRIPTOR.
	@param[out]      DescriptorVersion     A pointer to the location in which firmware returns the version number
	                                       associated with the EFI_MEMORY_DESCRIPTOR.

	@retval EFI_SUCCESS           The memory map was returned in the MemoryMap buffer.
	@retval EFI_BUFFER_TOO_SMALL  The MemoryMap buffer was too small. The current buffer size
	                              needed to hold the memory map is returned in MemoryMapSize.
	@retval EFI_INVALID_PARAMETER 1) MemoryMapSize is NULL.
	                              2) The MemoryMap buffer is not too small and MemoryMap is
	                                 NULL.

 **/
typedef
EFI_STATUS
(EFIAPI *EFI_GET_MEMORY_MAP)(
	IN OUT UINTN                 *MemoryMapSize,
	OUT    EFI_MEMORY_DESCRIPTOR *MemoryMap,
	OUT    UINTN                 *MapKey,
	OUT    UINTN                 *DescriptorSize,
	OUT    UINT32                *DescriptorVersion
);

/**
	Allocates pool memory.

	@param[in]   PoolType         The type of pool to allocate.
	                              MemoryType values in the range 0x70000000..0x7FFFFFFF
	                              are reserved for OEM use. MemoryType values in the range
	                              0x80000000..0xFFFFFFFF are reserved for use by UEFI OS loaders
	                              that are provided by operating system vendors.
	@param[in]   Size             The number of bytes to allocate from the pool.
	@param[out]  Buffer           A pointer to a pointer to the allocated buffer if the call succeeds;
	                              undefined otherwise.

	@retval EFI_SUCCESS           The requested number of bytes was allocated.
	@retval EFI_OUT_OF_RESOURCES  The pool requested could not be allocated.
	@retval EFI_INVALID_PARAMETER Buffer is NULL.
	                              PoolType is in the range EfiMaxMemoryType..0x6FFFFFFF.
	                              PoolType is EfiPersistentMemory.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_ALLOCATE_POOL)(
	IN  EFI_MEMORY_TYPE PoolType,
	IN  UINTN           Size,
	OUT VOID            **Buffer
);

/**
	Returns pool memory to the system.

	@param[in]  Buffer            The pointer to the buffer to free.

	@retval EFI_SUCCESS           The memory was returned to the system.
	@retval EFI_INVALID_PARAMETER Buffer was invalid.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_FREE_POOL)(
	IN VOID *Buffer
);

/**
	Terminates all boot services.

	@param[in]  ImageHandle       Handle that identifies the exiting image.
	@param[in]  MapKey            Key to the latest memory map.

	@retval EFI_SUCCESS           Boot services have been terminated.
	@retval EFI_INVALID_PARAMETER MapKey is incorrect.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_EXIT_BOOT_SERVICES)(
	IN EFI_HANDLE ImageHandle,
	IN UINTN      MapKey
);

#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL   0x00000001
#define EFI_OPEN_PROTOCOL_GET_PROTOCOL         0x00000002
#define EFI_OPEN_PROTOCOL_TEST_PROTOCOL        0x00000004
#define EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER  0x00000008
#define EFI_OPEN_PROTOCOL_BY_DRIVER            0x00000010
#define EFI_OPEN_PROTOCOL_EXCLUSIVE            0x00000020

/**
	Queries a handle to determine if it supports a specified protocol. If the protocol is supported by the
	handle, it opens the protocol on behalf of the calling agent.

	@param[in]   Handle           The handle for the protocol interface that is being opened.
	@param[in]   Protocol         The published unique identifier of the protocol.
	@param[out]  Interface        Supplies the address where a pointer to the corresponding Protocol
	                              Interface is returned.
	@param[in]   AgentHandle      The handle of the agent that is opening the protocol interface
	                              specified by Protocol and Interface.
  	param[in]   ControllerHandle If the agent that is opening a protocol is a driver that follows the
	                              UEFI Driver Model, then this parameter is the controller handle
	                              that requires the protocol interface. If the agent does not follow
	                              the UEFI Driver Model, then this parameter is optional and may
	                              be NULL.
	@param[in]   Attributes       The open mode of the protocol interface specified by Handle
	                              and Protocol.

	@retval EFI_SUCCESS           An item was added to the open list for the protocol interface, and the
	                              protocol interface was returned in Interface.
	@retval EFI_UNSUPPORTED       Handle does not support Protocol.
	@retval EFI_INVALID_PARAMETER One or more parameters are invalid.
	@retval EFI_ACCESS_DENIED     Required attributes can't be supported in current environment.
	@retval EFI_ALREADY_STARTED   Item on the open list already has requierd attributes whose agent
	                              handle is the same as AgentHandle.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_OPEN_PROTOCOL)(
	IN  EFI_HANDLE Handle,
	IN  EFI_GUID   *Protocol,
	OUT VOID       **Interface OPTIONAL,
	IN  EFI_HANDLE AgentHandle,
	IN  EFI_HANDLE ControllerHandle,
	IN  UINT32     Attributes
);

/**
	Closes a protocol on a handle that was opened using OpenProtocol().

	@param[in]  Handle            The handle for the protocol interface that was previously opened
	                              with OpenProtocol(), and is now being closed.
	@param[in]  Protocol          The published unique identifier of the protocol.
	@param[in]  AgentHandle       The handle of the agent that is closing the protocol interface.
	@param[in]  ControllerHandle  If the agent that opened a protocol is a driver that follows the
	                              UEFI Driver Model, then this parameter is the controller handle
	                              that required the protocol interface.

	@retval EFI_SUCCESS           The protocol instance was closed.
	@retval EFI_INVALID_PARAMETER 1) Handle is NULL.
	                              2) AgentHandle is NULL.
	                              3) ControllerHandle is not NULL and ControllerHandle is not a valid EFI_HANDLE.
	                              4) Protocol is NULL.
	@retval EFI_NOT_FOUND         1) Handle does not support the protocol specified by Protocol.
	                              2) The protocol interface specified by Handle and Protocol is not
	                                 currently open by AgentHandle and ControllerHandle.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_CLOSE_PROTOCOL)(
	IN EFI_HANDLE Handle,
	IN EFI_GUID   *Protocol,
	IN EFI_HANDLE AgentHandle,
	IN EFI_HANDLE ControllerHandle
);

///
/// EFI Boot Services Table.
///
typedef struct {
  ///
  /// The table header for the EFI Boot Services Table.
  ///
  EFI_TABLE_HEADER Hdr;

  //
  // Task Priority Services
  //
  VOID* RaiseTPL;
  VOID* RestoreTPL;

  //
  // Memory Services
  //
  VOID*              AllocatePages;
  VOID*              FreePages;
  EFI_GET_MEMORY_MAP GetMemoryMap;
  EFI_ALLOCATE_POOL  AllocatePool;
  EFI_FREE_POOL      FreePool;

  //
  // Event & Timer Services
  //
  VOID* CreateEvent;
  VOID* SetTimer;
  VOID* WaitForEvent;
  VOID* SignalEvent;
  VOID* CloseEvent;
  VOID* CheckEvent;

  //
  // Protocol Handler Services
  //
  VOID* InstallProtocolInterface;
  VOID* ReinstallProtocolInterface;
  VOID* UninstallProtocolInterface;
  VOID* HandleProtocol;
  VOID* Reserved;
  VOID* RegisterProtocolNotify;
  VOID* LocateHandle;
  VOID* LocateDevicePath;
  VOID* InstallConfigurationTable;

  //
  // Image Services
  //
  VOID*                  LoadImage;
  VOID*                  StartImage;
  VOID*                  Exit;
  VOID*                  UnloadImage;
  EFI_EXIT_BOOT_SERVICES ExitBootServices;

  //
  // Miscellaneous Services
  //
  VOID* GetNextMonotonicCount;
  VOID* Stall;
  VOID* SetWatchdogTimer;

  //
  // DriverSupport Services
  //
  VOID* ConnectController;
  VOID* DisconnectController;

  //
  // Open and Close Protocol Services
  //
  EFI_OPEN_PROTOCOL  OpenProtocol;
  EFI_CLOSE_PROTOCOL CloseProtocol;
  VOID*              OpenProtocolInformation;

  //
  // Library Services
  //
  VOID* ProtocolsPerHandle;
  VOID* LocateHandleBuffer;
  VOID* LocateProtocol;
  VOID*                    InstallMultipleProtocolInterfaces;
  VOID*                    UninstallMultipleProtocolInterfaces;

  //
  // 32-bit CRC Services
  //
  VOID* CalculateCrc32;

  //
  // Miscellaneous Services
  //
  VOID* CopyMem;
  VOID* SetMem;
  VOID* CreateEventEx;
} EFI_BOOT_SERVICES;

