/** @file

Copyright (c) 2017 - 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include <SaPolicyCommon.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/PchCycleDecodingLib.h>
#include <Library/PciLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseMemoryLib.h>

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
#include <Library/EcLib.h>
#include <EcCommands.h>

#define EC_INDEX_IO_PORT            0x1200
#define EC_INDEX_IO_HIGH_ADDR_PORT  EC_INDEX_IO_PORT+1
#define EC_INDEX_IO_LOW_ADDR_PORT   EC_INDEX_IO_PORT+2
#define EC_INDEX_IO_DATA_PORT       EC_INDEX_IO_PORT+3

#define ADC_3V_10BIT_GRANULARITY_MAX  (3005/1023)
#define PCB_VER_AD                    1
#define MODEL_ID_AD                   3

/**
  Read EC analog-digital converter.

  @param[out] DataBuffer

  @retval     EFI_SUCCESS       Command success
  @retval     EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
ReadEcAdcConverter (
  IN  UINT8        Adc,
  OUT UINT16       *DataBuffer
  )
{
  UINT8            AdcConvertersEnabled;  // Contains some ADCs and some DACs

  /* TODO: Separate into separate function of EC library? */
  // Backup enabled ADCs
  IoWrite8(EC_INDEX_IO_HIGH_ADDR_PORT, 0xff);
  IoWrite8(EC_INDEX_IO_LOW_ADDR_PORT, 0x15);  // ADDAEN
  AdcConvertersEnabled = IoRead8(EC_INDEX_IO_DATA_PORT);

  // Enable the desired ADC (not enabled by EC FW, not used by vendor FW)
  IoWrite8(EC_INDEX_IO_HIGH_ADDR_PORT, 0xff);
  IoWrite8(EC_INDEX_IO_LOW_ADDR_PORT, 0x15);  // ADDAEN
  IoWrite8(EC_INDEX_IO_DATA_PORT, AdcConvertersEnabled | ((1 << Adc) & 0xf));  // Field is bitmask

  // Sample the desired ADC
  IoWrite8(EC_INDEX_IO_HIGH_ADDR_PORT, 0xff);
  IoWrite8(EC_INDEX_IO_LOW_ADDR_PORT, 0x18);  // ADCTRL
  IoWrite8(EC_INDEX_IO_DATA_PORT, ((Adc << 1) & 0xf) | 1);  // Field is binary; OR the start bit

  // Read the desired ADC
  IoWrite8(EC_INDEX_IO_HIGH_ADDR_PORT, 0xff);
  IoWrite8(EC_INDEX_IO_LOW_ADDR_PORT, 0x19);  // ADCDAT
  *DataBuffer = IoRead8(EC_INDEX_IO_DATA_PORT) << 2;
  // Lower 2-bits of 10-bit ADC are in high bits of next register
  IoWrite8(EC_INDEX_IO_HIGH_ADDR_PORT, 0xff);
  IoWrite8(EC_INDEX_IO_LOW_ADDR_PORT, 0x1a);  // ECIF
  *DataBuffer |= IoRead8(EC_INDEX_IO_DATA_PORT) & 0xc0;

  // Restore enabled ADCs
  IoWrite8(EC_INDEX_IO_HIGH_ADDR_PORT, 0xff);
  IoWrite8(EC_INDEX_IO_LOW_ADDR_PORT, 0x15);  // ADDAEN
  IoWrite8(EC_INDEX_IO_DATA_PORT, AdcConvertersEnabled);

  return EFI_SUCCESS;
}

/**
  Get RVP3 board ID.
  There are 2 different RVP3 boards having different ID.
  This function will return board ID to caller.

  @param[out] DataBuffer

  @retval     EFI_SUCCESS       Command success
  @retval     EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
GetAspireVn7Dash572GBoardId (
  OUT UINT8    *BoardId
  )
{
  EFI_STATUS    Status;
  UINT16        DataBuffer;

  Status = ReadEcAdcConverter (MODEL_ID_AD, &DataBuffer);
  if (Status == EFI_SUCCESS) {
    // Board by max voltage range (of 10-bit, 3.005 V ADC)
    if (DataBuffer <= (1374/ADC_3V_10BIT_GRANULARITY_MAX)) {
      DEBUG ((DEBUG_ERROR, "BoardId is reserved?"));
    } else if (DataBuffer <= (2017/ADC_3V_10BIT_GRANULARITY_MAX)) {
      *BoardId = BoardIdNewgateSLx_dGPU;
    } else {
      *BoardId = BoardIdRayleighSLx_dGPU;
    }
    DEBUG ((DEBUG_INFO, "BoardId = 0x%X\n", *BoardId));
  }

  Status = ReadEcAdcConverter (PCB_VER_AD, &DataBuffer);
  if (Status == EFI_SUCCESS) {
    DEBUG ((DEBUG_INFO, "PCB version = 0x%X\n", DataBuffer));
  }
  return Status;
}

EFI_STATUS
EFIAPI
AspireVn7Dash572GBoardDetect (
  VOID
  )
{
  UINT8     BoardId;

  if (LibPcdGetSku () != 0) {
    return EFI_SUCCESS;
  }

  DEBUG ((DEBUG_INFO, "AspireVn7Dash572GDetectionCallback\n"));
  if (GetAspireVn7Dash572GBoardId (&BoardId) == EFI_SUCCESS) {
    if (BoardId == BoardIdRayleighSLx_dGPU) {
      LibPcdSetSku (BoardIdRayleighSLx_dGPU);
      ASSERT (LibPcdGetSku() == BoardIdRayleighSLx_dGPU);
    } else if (BoardId == BoardIdNewgateSLx_dGPU) {
      // TODO: Newgate is the "Black Edition" (VN7-792G). It uses PCH-H,
      //       should we forcibly halt execution here?
      LibPcdSetSku (BoardIdNewgateSLx_dGPU);
      ASSERT (LibPcdGetSku() == BoardIdNewgateSLx_dGPU);
    }
    DEBUG ((DEBUG_INFO, "SKU_ID: 0x%x\n", LibPcdGetSku()));
  }
  return EFI_SUCCESS;
}
