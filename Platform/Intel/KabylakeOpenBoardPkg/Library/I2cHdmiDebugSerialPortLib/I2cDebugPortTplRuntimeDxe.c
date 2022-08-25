/** @file
  Serial I/O Port library implementation for the HDMI I2C Debug Port
  DXE Library implementation

Copyright (c) 2022, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

STATIC EFI_TPL    mPreviousTpl        = 0;
STATIC EFI_EVENT  mEvent              = NULL;
STATIC UINT8      mEndOfBootServices  = 0;

/**
  Exit Boot Services Event notification handler.

  @param[in]  Event     Event whose notification function is being invoked
  @param[in]  Context   Pointer to the notification function's context

**/
VOID
EFIAPI
OnExitBootServices (
  IN      EFI_EVENT                 Event,
  IN      VOID                      *Context
  )
{
  mEndOfBootServices = 1;

  gBS->CloseEvent (Event);
}

/**
  For boot phases that utilize task priority levels (TPLs), this function raises
  the TPL to the appriopriate level needed to execute I/O to the I2C Debug Port
**/
VOID
RaiseTplForI2cDebugPortAccess (
  VOID
  )
{
  // DebugLibSerialPort exposes potential DEBUG bugs, such as early assertions
  if (gBS == NULL || mEndOfBootServices == 1) {
    return;
  }

  // An event is required for a boolean to bypass TPL modification after
  // exit-BS. RSC obviates this, requiring it for runtime DebugLibSerialPort.
  // - Consider creating a TplRuntimeDxe, although UefiRuntimeLib uses gST?
  // - BootScriptExecutorDxe is a special-case, where booleans are ineffective
  //
  // A constructor would cycle, SerialPortInitialize() takes no arguments,
  // and no BootServicesTableLib can be called by AutoGen early enough.
  // Therefore, we generate the event here.
  if (mEvent == NULL) {
    gBS->CreateEventEx (
           EVT_NOTIFY_SIGNAL,
           TPL_NOTIFY,
           OnExitBootServices,
           NULL,
           &gEfiEventExitBootServicesGuid,
           &mEvent
           );
  }

  if (EfiGetCurrentTpl () < TPL_NOTIFY) {
    mPreviousTpl = gBS->RaiseTPL (TPL_NOTIFY);
  }
}

/**
  For boot phases that utilize task priority levels (TPLs), this function
  restores the TPL to the previous level after I/O to the I2C Debug Port is
  complete
**/
VOID
RestoreTplAfterI2cDebugPortAccess (
  VOID
  )
{
  if (gBS == NULL || mEndOfBootServices == 1) {
    return;
  }

  if (mPreviousTpl > 0) {
    gBS->RestoreTPL (mPreviousTpl);
    mPreviousTpl = 0;
  }
}
