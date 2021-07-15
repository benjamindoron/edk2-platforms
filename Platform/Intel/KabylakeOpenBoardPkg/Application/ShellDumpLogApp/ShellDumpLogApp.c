/** @file
  This shell application will save the debug logs in the RSC infrastructure
  to files in the current directory (from memory)

  Copyright (c) 2008 - 2010, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiDxe.h>
#include <Uefi.h>
#include <Guid/MemoryStatusCodeRecord.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/ReportStatusCodeLib.h>
#include <Library/ShellLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiLib.h>

// TODO: Use Print() or the ConOut instance of DebugLib?
// FIXME: We could handle a timestamp/counter, but
//        making the user do it by argument is easier

STATIC CONST SHELL_PARAM_ITEM ParamList[] = {
  { L"-h", TypeFlag },   // -h   Help
  { L"-p", TypeValue },  // -p: PEI output file
  { L"-d", TypeValue },  // -p: DXE output file
  { NULL,  TypeMax }
};

/**
  Entry point for this application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                        Status;
  LIST_ENTRY                        *ParamPackage;
  CHAR16                            *ProblemParam = NULL;
  BOOLEAN                           HelpFlag;
  CONST CHAR16                      *PeiOutputFileName = NULL;
  CONST CHAR16                      *DxeOutputFileName = NULL;
  EFI_PEI_HOB_POINTERS              Hob;
  MEMORY_STATUSCODE_PACKET_HEADER   *PacketHeader;
  MEMORY_STATUSCODE_RECORD          *Record;
  SHELL_FILE_HANDLE                 FileHandle = NULL;
  UINTN                             *BufferSize;
//  UINT8                             Counter = 0;
  RUNTIME_MEMORY_STATUSCODE_HEADER  *RtMemoryStatusCodeTable;

  Status = ShellCommandLineParseEx(ParamList, &ParamPackage, &ProblemParam, FALSE, TRUE);
  if (EFI_ERROR(Status)) {
    if (ProblemParam != NULL) {
      Print(L"Invalid parameter %s\n", ProblemParam);
      FreePool(ProblemParam);
    } else {
      Print(L"Unable to parse command line. Code=%r", Status);
    }
    return SHELL_INVALID_PARAMETER;
  }

  HelpFlag  = ShellCommandLineGetFlag(ParamPackage, L"-h");
  PeiOutputFileName = ShellCommandLineGetValue(ParamPackage, L"-p");
  DxeOutputFileName = ShellCommandLineGetValue(ParamPackage, L"-d");

  if (PeiOutputFileName == NULL && DxeOutputFileName == NULL) {
    Print(L"Must at least specify output destination for PEI or DXE!");
    HelpFlag = TRUE;
  }
  
  if (HelpFlag) {
    AsciiPrint("%a [-p PeiOutputFileName] [-d DxeOutputFileName] [-h]\n", gEfiCallerBaseName);
    return EFI_SUCCESS;
  }

  // Dump PEI phase logs - anticipating only one HOB
  Hob.Raw = GetFirstGuidHob(&gMemoryStatusCodeRecordGuid);
  if (Hob.Raw != NULL && PeiOutputFileName != NULL) {
    Print(L"Debug logs from PEI phase found, proceeding...");
    // Advance past the header
    PacketHeader = (MEMORY_STATUSCODE_PACKET_HEADER *) GET_GUID_HOB_DATA (Hob.Guid);
    Record = (MEMORY_STATUSCODE_RECORD *) (PacketHeader + 1);
    // Just drop the end of the buffer
    /* PcdGet16(PcdStatusCodeMemorySize)*1024 + sizeof(MEMORY_STATUSCODE_PACKET_HEADER)*/
    *BufferSize = 1 * 1024;

    // Execute binary copy until proper parsing is determined - TODO: Factor into helper?
    Status = ShellOpenFileByName(PeiOutputFileName, &FileHandle, EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE, 0);
    if (!EFI_ERROR(Status)) {
      Status = ShellWriteFile(FileHandle, BufferSize, Record);
      if (!EFI_ERROR(Status)) {
        Status = ShellCloseFile(&FileHandle);
        if (EFI_ERROR(Status)) {
          Print(L"Unable to close destination file!");
        }
        FileHandle = NULL;
      } else {
        Print(L"Unable to write destination file!");
      }
    } else {
      Print(L"Unable to open destination file!");
    }
  } else {
    Print(L"Debug logs from PEI phase NOT found, skipping...");
  }

  // Dump DXE phase logs - anticipating only one entry in config table
  EfiGetSystemConfigurationTable(&gMemoryStatusCodeRecordGuid, (VOID **) &RtMemoryStatusCodeTable);
  if (RtMemoryStatusCodeTable != NULL && DxeOutputFileName != NULL) {
    Print(L"Debug logs from DXE phase found, proceeding...");
    // Advance past the header
    Record = (MEMORY_STATUSCODE_RECORD *) (RtMemoryStatusCodeTable + 1);
    // Just drop the end of the buffer
    /* PcdGet16 PcdStatusCodeMemorySize)*1024 + sizeof(RUNTIME_MEMORY_STATUSCODE_HEADER)*/
    *BufferSize = 128 * 1024;

    // Execute binary copy until proper parsing is determined - TODO: Factor into helper?
    Status = ShellOpenFileByName(DxeOutputFileName, &FileHandle, EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE, 0);
    if (!EFI_ERROR(Status)) {
      Status = ShellWriteFile(FileHandle, BufferSize, RtMemoryStatusCodeTable);
      if (!EFI_ERROR(Status)) {
        Status = ShellCloseFile(&FileHandle);
        if (EFI_ERROR(Status)) {
          Print(L"Unable to close destination file!");
        }
        FileHandle = NULL;
      } else {
        Print(L"Unable to write destination file!");
      }
    } else {
      Print(L"Unable to open destination file!");
    }
  } else {
    Print(L"Debug logs from DXE phase NOT found, skipping...");
  }

  Print(L"Unable to retrieve SMM debug logs at this time!");

  return EFI_SUCCESS;
}
