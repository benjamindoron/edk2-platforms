/** @file
  Common EC commands.

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>
#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/EcLib.h>

/**
  Read a byte of EC RAM.

  @param[in]  Address          Address to read
  @param[out] Data             Data received

  @retval    EFI_SUCCESS       Command success
  @retval    EFI_DEVICE_ERROR  Command error
  @retval    EFI_TIMEOUT       Command timeout
**/
EFI_STATUS
EcRead (
  IN  UINT8                  Address,
  OUT UINT8                  *Data
  )
{
  EFI_STATUS                 Status;

  Status = SendEcCommand(0x80);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "%s: SendEcCommand(0x80) failed!\n", __func__));
    return Status;
  }

  Status = SendEcData(Address);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "%s: SendEcData(Address) failed!\n", __func__));
    return Status;
  }

  Status = ReceiveEcData(Data);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "%s: ReceiveEcData(Data) failed!\n", __func__));
    return Status;
  }
  return EFI_SUCCESS;
}

/**
  Write a byte of EC RAM.

  @param[in] Address           Address to write
  @param[in] Data              Data to write

  @retval    EFI_SUCCESS       Command success
  @retval    EFI_DEVICE_ERROR  Command error
  @retval    EFI_TIMEOUT       Command timeout
**/
EFI_STATUS
EcWrite (
  IN  UINT8                  Address,
  IN  UINT8                  Data
  )
{
  EFI_STATUS                 Status;

  Status = SendEcCommand(0x81);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "%s: SendEcCommand(0x81) failed!\n", __func__));
    return Status;
  }

  Status = SendEcData(Address);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "%s: SendEcData(Address) failed!\n", __func__));
    return Status;
  }

  Status = SendEcData(Data);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "%s: SendEcData(Data) failed!\n", __func__));
    return Status;
  }
  return EFI_SUCCESS;
}
