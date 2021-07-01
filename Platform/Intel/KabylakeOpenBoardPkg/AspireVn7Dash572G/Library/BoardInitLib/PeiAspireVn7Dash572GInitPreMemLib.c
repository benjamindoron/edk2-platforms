/** @file

Copyright (c) 2017 - 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <SaPolicyCommon.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BoardEcLib.h>
#include <Library/EcLib.h>
#include <Library/IoLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/PchCycleDecodingLib.h>
#include <Library/PciLib.h>
#include <Library/PcdLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/TimerLib.h>

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
#include <Library/SiliconInitLib.h>

#include "PeiAspireVn7Dash572GInitLib.h"

#include <ConfigBlock.h>
#include <ConfigBlock/MemoryConfig.h>

#ifndef STALL_ONE_MILLI_SECOND
#define STALL_ONE_MILLI_SECOND  1000
#endif

//
// Reference RCOMP resistors on motherboard - for Aspire VN7-572G
//
GLOBAL_REMOVE_IF_UNREFERENCED const UINT16 RcompResistorAspireVn7Dash572G[SA_MRC_MAX_RCOMP] = { 121, 80, 100 };
//
// RCOMP target values for RdOdt, WrDS, WrDSCmd, WrDSCtl, WrDSClk - for Aspire VN7-572G
//
GLOBAL_REMOVE_IF_UNREFERENCED const UINT16 RcompTargetAspireVn7Dash572G[SA_MRC_MAX_RCOMP_TARGETS] = { 100, 40, 40, 23, 40 };

//
// dGPU power GPIO definitions
#define DGPU_PRESENT	GPIO_SKL_LP_GPP_A20	/* Active low */
#define DGPU_HOLD_RST	GPIO_SKL_LP_GPP_B4	/* Active low */
#define DGPU_PWR_EN	GPIO_SKL_LP_GPP_B21	/* Active low */

/**
  SkylaeA0Rvp3 board configuration init function for PEI pre-memory phase.

  PEI_BOARD_CONFIG_PCD_INIT

  @param  Content  pointer to the buffer contain init information for board init.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_INVALID_PARAMETER   The parameter is NULL.
**/
EFI_STATUS
EFIAPI
AspireVn7Dash572GInitPreMem (
  VOID
  )
{
  //
  // HSIO PTSS Table
  //
  PcdSet32S (PcdSpecificLpHsioPtssTable1,     (UINTN) PchLpHsioPtss_Bx_AspireVn7Dash572G);
  PcdSet16S (PcdSpecificLpHsioPtssTable1Size, (UINTN) PchLpHsioPtss_Bx_AspireVn7Dash572G_Size);
  PcdSet32S (PcdSpecificLpHsioPtssTable2,     (UINTN) PchLpHsioPtss_Cx_AspireVn7Dash572G);
  PcdSet16S (PcdSpecificLpHsioPtssTable2Size, (UINTN) PchLpHsioPtss_Cx_AspireVn7Dash572G_Size);

  //
  // DRAM related definition
  //
  PcdSet8S (PcdSaMiscUserBd, 5);  // ULT/ULX/Mobile Halo
  PcdSet8S (PcdMrcCaVrefConfig, 2);  // "VREF_CA to CH_A and VREF_DQ_B to CH_B" - for DDR4 boards
  PcdSetBoolS (PcdMrcDqPinsInterleaved, TRUE);

  PcdSet32S (PcdMrcRcompResistor, (UINTN) RcompResistorAspireVn7Dash572G);
  PcdSet32S (PcdMrcRcompTarget, (UINTN) RcompTargetAspireVn7Dash572G);
  // TODO: Sample policy will populate Dq/Dqs, we should override (but "0" will cause
  //       `if (Buffer)` to fail...)
  //
  // Example policy for DIMM slots implementation boards:
  // 1. Assign Smbus address of DIMMs and SpdData will be updated later
  //    by reading from DIMM SPD.
  // 2. No need to apply hardcoded SpdData buffers here for such board.
  //   Example:
  //   PcdMrcSpdAddressTable0 = 0xA0
  //   PcdMrcSpdAddressTable1 = 0xA2
  //   PcdMrcSpdAddressTable2 = 0xA4
  //   PcdMrcSpdAddressTable3 = 0xA6
  //   PcdMrcSpdData = 0
  //   PcdMrcSpdDataSize = 0
  //
  PcdSet8S (PcdMrcSpdAddressTable0, 0xA0);
  PcdSet8S (PcdMrcSpdAddressTable1, 0);
  PcdSet8S (PcdMrcSpdAddressTable2, 0xA4);
  PcdSet8S (PcdMrcSpdAddressTable3, 0);
  PcdSet32S (PcdMrcSpdData, 0);
  PcdSet16S (PcdMrcSpdDataSize, 0);

  return EFI_SUCCESS;
}

/**
  Configures GPIO.

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
  Configure GPIO Before Memory is not ready.

**/
VOID
GpioInitPreMem (
  VOID
  )
{
  ConfigureGpio (mGpioTableAspireVn7Dash572G_early, mGpioTableAspireVn7Dash572G_earlySize);
}

/**
  Init based on PeiOemModule. KbcPeim does not appear to be used.
  It implements commands also found in RtKbcDriver and SmmKbcDriver.

**/
VOID
EcInit (
  VOID
  )
{
  EFI_BOOT_MODE  BootMode;
  UINT8          PowerState;
  UINT8          OutData;
  UINT32         GpeSts;

  /* This is called via a "$FNC" in a PeiOemModule pointer table */
  IoWrite8(0x6C, 0x5A);  // 6Ch is the EC sideband port
  PeiServicesGetBootMode(&BootMode);
  if (BootMode == BOOT_ON_S3_RESUME) {
    /* "MLID" in LGMR-based memory map is equivalent to "ELID" in EC-based
     * memory map. Vendor firmware accesses through LGMR; remapped */
    EcRead(0x70, &PowerState);
    if (!(PowerState & 2)) {  // Lid is closed
      EcCmd90Read(0x0A, &OutData);  // Code executed, do not remap
      if (!(OutData & 2))
        EcCmd91Write(0x0A, OutData | 2);  // Code executed, do not remap

      /* TODO: Clear events and go back to sleep */
 //     pmc_clear_pm1_status();
      /* Clear GPE0_STS[127:96] */
//      GpeSts = inl(ACPI_BASE_ADDRESS + GPE0_STS(3));
//      outl(GpeSts, ACPI_BASE_ADDRESS + GPE0_STS(3));
      /* TODO: Clear xHCI PM_CS[PME_Status] - 74h[15]? */

//      pmc_enable_pm1_control(SLP_EN | (SLP_TYP_S3 << SLP_TYP_SHIFT));
//      halt();
    }
  }
}

/**
  Initialises the dGPU.

**/
VOID
DgpuPowerOn (
  VOID
  )
{
  UINT32         OutputVal;

  GpioGetOutputValue(DGPU_PRESENT, &OutputVal);
  if (!OutputVal) {
    GpioSetOutputValue(DGPU_HOLD_RST, 0);  // Assert dGPU_HOLD_RST#
    MicroSecondDelay(2 * STALL_ONE_MILLI_SECOND);
    GpioSetOutputValue(DGPU_PWR_EN, 0);  // Assert dGPU_PWR_EN#
    MicroSecondDelay(7 * STALL_ONE_MILLI_SECOND);
    GpioSetOutputValue(DGPU_HOLD_RST, 1);  // Deassert dGPU_HOLD_RST#
    MicroSecondDelay(30 * STALL_ONE_MILLI_SECOND);
  } else {
    GpioSetOutputValue(DGPU_HOLD_RST, 0);  // Assert dGPU_HOLD_RST#
    GpioSetOutputValue(DGPU_PWR_EN, 1);  // Deassert dGPU_PWR_EN#
  }
}

/**
  Configure LPC.
  TODO: Execute even earlier, so that EC (index) is available
        for the ADC reads in board detection (it seems to work)?

**/
VOID
LpcInit (
  VOID
  )
{
  //
  // Enable I/O decoding for COM1(3F8h-3FFh), COM2(2F8h-2FFh), I/O port 2Eh/2Fh, 4Eh/4Fh, 60h/64Fh and 62h/66h.
  //
  PchLpcIoDecodeRangesSet (PcdGet16 (PcdLpcIoDecodeRange));
  PchLpcIoEnableDecodingSet (PcdGet16 (PchLpcIoEnableDecoding));

  //
  // Program and Enable EC (sideband) Port Addresses and range
  //
  PchLpcGenIoRangeSet (0x68, 0x08);

  //
  // Program and Enable EC (index) Port Addresses and range
  //
  PchLpcGenIoRangeSet (0x1200, 0x10);
}

/**
  Configure GPIO and SIO before memory ready.

  @retval  EFI_SUCCESS   Operation success.
**/
EFI_STATUS
EFIAPI
AspireVn7Dash572GBoardInitBeforeMemoryInit (
  VOID
  )
{
  EcInit ();
  GpioInitPreMem ();
  DgpuPowerOn ();
  AspireVn7Dash572GInitPreMem ();

  LpcInit ();
    
  ///
  /// Do basic PCH init
  ///
  SiliconInit ();

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
AspireVn7Dash572GBoardDebugInit (
  VOID
  )
{
  ///
  /// Do Early PCH init
  ///
  EarlySiliconInit ();
  return EFI_SUCCESS;
}

EFI_BOOT_MODE
EFIAPI
AspireVn7Dash572GBoardBootModeDetect (
  VOID
  )
{
  // TODO: How is crisis recovery mode detected by vendor?
  return BOOT_WITH_FULL_CONFIGURATION;
}

