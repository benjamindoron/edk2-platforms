/** @file
  Kaby Lake RVP 3 Board Initialization Pre-Memory library

Copyright (c) 2017 - 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/BoardInitLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <PlatformBoardId.h>
//rm
#include <Library/CacheAsRamLib.h>
#include <Library/MtrrLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Register/Cpuid.h>
#include <Register/Msr.h>
#include <Library/FspPlatformLib.h>
#include <Library/BaseMemoryLib.h>

EFI_STATUS
EFIAPI
AspireVn7Dash572GBoardDetect (
  VOID
  );

EFI_BOOT_MODE
EFIAPI
AspireVn7Dash572GBoardBootModeDetect (
  VOID
  );

EFI_STATUS
EFIAPI
AspireVn7Dash572GBoardDebugInit (
  VOID
  );

EFI_STATUS
EFIAPI
AspireVn7Dash572GBoardInitBeforeMemoryInit (
  VOID
  );

EFI_STATUS
EFIAPI
BoardDetect (
  VOID
  )
{
  AspireVn7Dash572GBoardDetect ();
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
BoardDebugInit (
  VOID
  )
{
  AspireVn7Dash572GBoardDebugInit ();
  return EFI_SUCCESS;
}

EFI_BOOT_MODE
EFIAPI
BoardBootModeDetect (
  VOID
  )
{
  return AspireVn7Dash572GBoardBootModeDetect ();
}

EFI_STATUS
EFIAPI
BoardInitBeforeMemoryInit (
  VOID
  )
{
  AspireVn7Dash572GBoardInitBeforeMemoryInit ();
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
BoardInitAfterMemoryInit (
  VOID
  )
{
  // TODO: Set-up LGMR
  DEBUG((DEBUG_INFO, "after memory init\n"));

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
BoardInitBeforeTempRamExit (
  VOID
  )
{
  DEBUG((DEBUG_INFO, "before CAR teardown\n"));

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
BoardInitAfterTempRamExit (
  VOID
  )
{
  /* REDACTED? */

  return EFI_SUCCESS;
}

