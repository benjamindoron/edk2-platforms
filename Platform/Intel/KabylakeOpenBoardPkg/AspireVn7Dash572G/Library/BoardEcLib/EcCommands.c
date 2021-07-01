/** @file
  Board EC commands.

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>
#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/EcLib.h>

/* TODO - Implement:
 *   - Commands: 0x58, 0xE1 and 0xE2
 *     - 0x51, 0x52: EC flash write?
 *   - ACPI CMDB: 0x63 and 0x64, 0xC7
 *     - 0x0B: Flash write (Boolean argument? Set in offset 0x0B?)
 *
 * NB: Consider that if UEFI driver consumes
 *     unimplemented PPI/protocol, the driver is dead code.
 *
 * NOTE: Check protocol use.
 *   - Commands delivered across modules
 *   - EC writes also control behaviour
 */

/**
  Reads a byte of EC RAM.

  @param[in]  Address          Address to read
  @param[out] Data             Data received

  @retval    EFI_SUCCESS       Command success
  @retval    EFI_DEVICE_ERROR  Command error
  @retval    EFI_TIMEOUT       Command timeout
**/
EFI_STATUS
EcCmd90Read (
  IN  UINT8                  Address,
  OUT UINT8                  *Data
  )
{
  EFI_STATUS                 Status;

  Status = SendEcCommand(0x90);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "%s: SendEcCommand(0x90) failed!\n", __func__));
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
  Writes a byte of EC RAM.

  @param[in] Address           Address to write
  @param[in] Data              Data to write

  @retval    EFI_SUCCESS       Command success
  @retval    EFI_DEVICE_ERROR  Command error
  @retval    EFI_TIMEOUT       Command timeout
**/
EFI_STATUS
EcCmd91Write (
  IN  UINT8                  Address,
  IN  UINT8                  Data
  )
{
  EFI_STATUS                 Status;

  Status = SendEcCommand(0x91);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "%s: SendEcCommand(0x91) failed!\n", __func__));
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

/**
  Query the EC status.

  @param[out] Status           EC status byte

  @retval    EFI_SUCCESS       Command success
  @retval    EFI_DEVICE_ERROR  Command error
  @retval    EFI_TIMEOUT       Command timeout
**/
EFI_STATUS
EcCmd94Query (
  OUT UINT8                  *Data
  )
{
  EFI_STATUS                 Status;

  Status = SendEcCommand(0x94);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "%s: SendEcCommand(0x94) failed!\n", __func__));
    return Status;
  }

  Status = ReceiveEcData(Data);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "%s: ReceiveEcData(Data) failed!\n", __func__));
    return Status;
  }
  return EFI_SUCCESS;
}
