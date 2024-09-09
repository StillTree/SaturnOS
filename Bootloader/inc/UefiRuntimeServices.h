#pragma once

#include "UefiTypes.h"

///
/// EFI Runtime Services Table.
///
typedef struct {
  ///
  /// The table header for the EFI Runtime Services Table.
  ///
  EFI_TABLE_HEADER Hdr;

  //
  // Time Services
  //
  VOID* GetTime;
  VOID* SetTime;
  VOID* GetWakeupTime;
  VOID* SetWakeupTime;

  //
  // Virtual Memory Services
  //
  VOID* SetVirtualAddressMap;
  VOID* ConvertPointer;

  //
  // Variable Services
  //
  VOID* GetVariable;
  VOID* GetNextVariableName;
  VOID* SetVariable;

  //
  // Miscellaneous Services
  //
  VOID* GetNextHighMonotonicCount;
  VOID* ResetSystem;

  //
  // UEFI 2.0 Capsule Services
  //
  VOID* UpdateCapsule;
  VOID* QueryCapsuleCapabilities;

  //
  // Miscellaneous UEFI 2.0 Service
  //
  VOID* QueryVariableInfo;
} EFI_RUNTIME_SERVICES;

