/** @file
  Acer Aspire VN7-572G SMM Board ACPI Enable library

Copyright (c) 2017 - 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>
#include <Uefi.h>
#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/EcLib.h>
#include <Library/IoLib.h>
#include <Library/BoardAcpiTableLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>

#include <PlatformBoardId.h>

EFI_STATUS
EFIAPI
AspireVn7Dash572GBoardEnableAcpi (
  IN BOOLEAN  EnableSci
  )
{
  EFI_STATUS  Status;

  /* Tests at runtime show this re-enables charging and battery reporting */
  Status = SendEcCommand(0xE9);  /* Vendor implements using ACPI "CMDB" register" */
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "%s: SendEcCommand(0xE9) failed!\n", __func__));
    return EFI_DEVICE_ERROR;
  }

  Status = SendEcData(0x81);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "%s: SendEcData(0x81) failed!\n", __func__));
    return EFI_DEVICE_ERROR;
  }

  /* TODO: Set touchpad GPP owner to ACPI? */

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
AspireVn7Dash572GBoardDisableAcpi (
  IN BOOLEAN  DisableSci
  )
{
  EFI_STATUS  Status;

  /* Tests at runtime show this disables charging and battery reporting */
  Status = SendEcCommand(0xE9);  /* Vendor implements using ACPI "CMDB" register" */
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "%s: SendEcCommand(0xE9) failed!\n", __func__));
    return EFI_DEVICE_ERROR;
  }

  Status = SendEcData(0x80);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "%s: SendEcData(0x80) failed!\n", __func__));
    return EFI_DEVICE_ERROR;
  }

  /* TODO: Set touchpad GPP owner to GPIO? */

  return EFI_SUCCESS;
}

