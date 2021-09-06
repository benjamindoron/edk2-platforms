/** @file
  Aspire VN7-572G Board Initialization DXE library

  Copyright (c) 2021, Baruch Binyamin Doron
  Copyright (c) 2021, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiDxe.h>
#include <Library/BoardEcLib.h>
#include <Library/BoardInitLib.h>
#include <Library/DebugLib.h>
#include <Library/EcLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/ResetNotification.h>

EFI_RESET_NOTIFICATION_PROTOCOL  *mResetNotify = NULL;

/**
  Update the EC's clock.

**/
VOID
EFIAPI
EcSendTime (
  VOID
  )
{
  EFI_STATUS  Status;
  EFI_TIME    EfiTime;
  // Time could be negative (before 2016)
  INT32       EcTime;
  UINT8       EcTimeByte;
  INTN        Index;
  UINT8       EcResponse;

  DEBUG ((DEBUG_INFO, "%a() Starts\n", __FUNCTION__));

  Status = gRT->GetTime (&EfiTime, NULL);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Failed to retrieve current time\n"));
    return;
  }

  // Time since year of release. Note that "century" is ignored.
  EcTime = ((EfiTime.Year << 26) + (EfiTime.Month << 22) + (EfiTime.Day << 17)
         + (EfiTime.Hour << 12) + (EfiTime.Minute << 6) + (EfiTime.Second)
         /* 16 years */
         - 0x40000000);

  DEBUG ((DEBUG_INFO, "EC: reporting present time 0x%x\n", EcTime));
  SendEcCommand (0xE0);
  for (Index = 0; Index < 4; Index++) {
    // Shift bytes
    EcTimeByte = (UINT8) (EcTime >> (Index * 8));
    DEBUG ((DEBUG_INFO, "EC: Sending 0x%x (iteration %d)\n", EcTimeByte, Index));
    SendEcData (EcTimeByte);
  }

  Status = ReceiveEcData (&EcResponse);
  if (!EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "EC: response 0x%x\n", EcResponse));
  }

  DEBUG ((DEBUG_INFO, "%a() Ends\n", __FUNCTION__));
}

/**
  Process an EC time request.

**/
VOID
EFIAPI
EcRequestsTime (
  VOID
  )
{
  UINT8           Dat;

  DEBUG ((DEBUG_INFO, "%a() Starts\n", __FUNCTION__));

  /* This is executed as protocol notify in vendor's RtKbcDriver when *CommonService
   * protocol is installed. Effectively, this code could execute from the entrypoint */
  EcCmd90Read (0x79, &Dat);
  if (Dat & BIT0) {
    EcSendTime ();
  }

  DEBUG ((DEBUG_INFO, "%a() Ends\n", __FUNCTION__));
}

/**
  Notify EC of reset events.

  @param[in] ResetType    The type of reset to perform.
  @param[in] ResetStatus  The status code for the reset.
  @param[in] DataSize     The size, in bytes, of ResetData.
  @param[in] ResetData    For a ResetType of EfiResetCold, EfiResetWarm, or EfiResetShutdown
                          the data buffer starts with a Null-terminated string, optionally
                          followed by additional binary data. The string is a description
                          that the caller may use to further indicate the reason for the
                          system reset. For a ResetType of EfiResetPlatformSpecific the data
                          buffer also starts with a Null-terminated string that is followed
                          by an EFI_GUID that describes the specific type of reset to
                          perform.

**/
VOID
EFIAPI
EcResetSystemHook (
  IN EFI_RESET_TYPE           ResetType,
  IN EFI_STATUS               ResetStatus,
  IN UINTN                    DataSize,
  IN VOID                     *ResetData OPTIONAL
  )
{
  // If boolean PCD tokens 0xBD, 0xBE and 0xBF are set in vendor FW,
  // OEM also sends command 0x5A with argument 0xAA via ACPI "CMDB" method and stalls for
  // 100000, then sets ResetType to EfiResetShutdown.
  // PCD token 0xBF may be set in a separate function of DxeOemDriver if
  // some bits of EC RAM offset 0x5E are set.
  // TODO: More information is needed
  if (ResetType == EfiResetShutdown) {
    EcCmd91Write (0x76, 7);  // "ECSS" register
    // TODO: Write twice, like OEM?
    EcCmd91Write (0x76, 7);  // "ECSS" register
    // Now OEM calls function offset 2 in ACER_BOOT_DEVICE_SERVICE_PROTOCOL_GUID.
    // TODO: What does this do?
  }
}

/**
  A hook for board-specific initialization after PCI enumeration.

  @retval EFI_SUCCESS   The board initialization was successful.
  @retval EFI_NOT_READY The board has not been detected yet.
**/
EFI_STATUS
EFIAPI
BoardInitAfterPciEnumeration (
  VOID
  )
{
  EFI_STATUS                       Status;

  DEBUG ((DEBUG_INFO, "%a() Starts\n", __FUNCTION__));

  // Send EC the present time, if requested
  EcRequestsTime ();

  // Add a callback to gRT->ResetSystem() to notify EC, rather than hooking the table,
  // (as vendor's DxeOemDriver does)
  Status = gBS->LocateProtocol (&gEfiResetNotificationProtocolGuid, NULL, (VOID **) &mResetNotify);
  if (!EFI_ERROR (Status)) {
    Status = mResetNotify->RegisterResetNotify (mResetNotify, EcResetSystemHook);
    ASSERT_EFI_ERROR (Status);
    DEBUG ((DEBUG_INFO, "EC: Added callback to notify EC of resets\n"));
  }

  DEBUG ((DEBUG_INFO, "%a() Ends\n", __FUNCTION__));
  return EFI_SUCCESS;
}

/**
  A hook for board-specific functionality for the ReadyToBoot event.

  @retval EFI_SUCCESS   The board initialization was successful.
  @retval EFI_NOT_READY The board has not been detected yet.
**/
EFI_STATUS
EFIAPI
BoardInitReadyToBoot (
  VOID
  )
{
  return EFI_SUCCESS;
}

/**
  A hook for board-specific functionality for the ExitBootServices event.

  @retval EFI_SUCCESS   The board initialization was successful.
  @retval EFI_NOT_READY The board has not been detected yet.
**/
EFI_STATUS
EFIAPI
BoardInitEndOfFirmware (
  VOID
  )
{
  EFI_STATUS                       Status;

  DEBUG ((DEBUG_INFO, "%a() Starts\n", __FUNCTION__));

  // Remove ResetSystem callback. ACPI will be notifying EC of events
  if (mResetNotify != NULL) {
    Status = mResetNotify->UnregisterResetNotify (mResetNotify, EcResetSystemHook);
    ASSERT_EFI_ERROR (Status);
    DEBUG ((DEBUG_INFO, "EC: Removed callback to notify EC of resets\n"));
  }

  DEBUG ((DEBUG_INFO, "%a() Ends\n", __FUNCTION__));
  return EFI_SUCCESS;
}
