/** @file

Copyright (c) 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <SaPolicyCommon.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BoardEcLib.h>
#include <Library/IoLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/PchCycleDecodingLib.h>
#include <Library/PciLib.h>
#include <Library/PeiSaPolicyLib.h>
#include <Library/BoardInitLib.h>
#include <PchAccess.h>
#include <Library/GpioNativeLib.h>
#include <Library/GpioLib.h>
#include <GpioPinsSklLp.h>
#include <GpioPinsSklH.h>
#include <Library/GpioExpanderLib.h>
#include <SioRegs.h>
#include <Library/PchPcrLib.h>
#include <IoExpander.h>
#include <Library/PcdLib.h>
#include <Library/SiliconInitLib.h>

#include "PeiAspireVn7Dash572GInitLib.h"

/**
  SkylaeA0Rvp3 board configuration init function for PEI post memory phase.

  PEI_BOARD_CONFIG_PCD_INIT

  @param  Content  pointer to the buffer contain init information for board init.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   The parameter is NULL.
**/
EFI_STATUS
EFIAPI
AspireVn7Dash572GInit (
  VOID
  )
{
  PcdSet32S (PcdHdaVerbTable, (UINTN) &HdaVerbTableAlc255AspireVn7Dash572G);
  PcdSet32S (PcdDisplayAudioHdaVerbTable, (UINTN) &HdaVerbTableDisplayAudio);

  return EFI_SUCCESS;
}

/**
  Configures GPIO

  @param[in]  GpioTable       Point to Platform Gpio table
  @param[in]  GpioTableCount  Number of Gpio table entries

**/
VOID
ConfigureGpio (
  IN GPIO_INIT_CONFIG                 *GpioDefinition,
  IN UINT16                           GpioTableCount
  )
{
  EFI_STATUS          Status;

  DEBUG ((DEBUG_INFO, "ConfigureGpio() Start\n"));

  Status = GpioConfigurePads (GpioTableCount, GpioDefinition);

  DEBUG ((DEBUG_INFO, "ConfigureGpio() End\n"));
}


/**
  Configure GPIO

**/
VOID
GpioInitPostMem (
  VOID
  )
{
  ConfigureGpio (mGpioTableAspireVn7Dash572G, mGpioTableAspireVn7Dash572GSize);
}

VOID
ec_fills_time (
  VOID
  )
{
#if 0
  struct rtc_time time;
  rtc_get(&time);

  u8 ec_time_byte;
  int ec_time = ((time.year << 26) + (time.mon << 22) + (time.mday << 17)
      + (time.hour << 12) + (time.min << 6) + (time.sec)
      /* 16 years */
      - 0x40000000);

  printk(BIOS_DEBUG, "EC: reporting present time 0x%x\n", ec_time);
  send_ec_command(0xE0);
  for (int i = 0; i < 4; i++) {
    ec_time_byte = ec_time >> (i*sizeof(ec_time_byte));
    printk(BIOS_DEBUG, "EC: Sending 0x%x (iteration %d)\n", ec_time_byte, i);
    send_ec_data(ec_time_byte);
  }

  printk(BIOS_DEBUG, "EC: response 0x%x\n", recv_ec_data());
#endif
}

/**
  Configure EC
  FIXME: Should actually be DXE phase (RTC is unavailable?)

**/
VOID
EcInit (
  VOID
  )
{
  UINT8           Dat;
  /* UEFI modules "notify" this protocol in RtKbcDriver */
  EcCmd90Read(0x79, &Dat);
  if (Dat & 1)
    ec_fills_time();
}


/**
  Configure GPIO and SIO

  @retval  EFI_SUCCESS   Operation success.
**/
EFI_STATUS
EFIAPI
AspireVn7Dash572GBoardInitBeforeSiliconInit (
  VOID
  )
{
  AspireVn7Dash572GInit ();

  GpioInitPostMem ();
  EcInit ();
    
  ///
  /// Do Late PCH init
  ///
  LateSiliconInit ();

  return EFI_SUCCESS;
}
