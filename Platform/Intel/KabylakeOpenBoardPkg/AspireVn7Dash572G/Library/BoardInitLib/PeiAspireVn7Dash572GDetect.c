/** @file

Copyright (c) 2017 - 2021, Intel Corporation. All rights reserved.<BR>
Copyright (c) 2021, Baruch Binyamin Doron<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>
#include "PeiAspireVn7Dash572GInitLib.h"
#include <Library/BoardEcLib.h>
#include <Library/DebugLib.h>

#define ADC_3V_10BIT_GRANULARITY_MAX  (3005 / 1023)
#define PCB_VER_AD                    1
#define MODEL_ID_AD                   3

/**
  Get Aspire V Nitro (Skylake) board ID. There are 3 different boards
  having different PCH (therefore, ID). This function will return board ID to caller.
  - TODO/NB: Detection is still unreliable. Likely must await interrupt.

  @param[out] DataBuffer

  @retval     EFI_SUCCESS       Command success
  @retval     EFI_DEVICE_ERROR  Command error
**/
VOID
GetAspireVn7Dash572GBoardId (
  OUT UINT8    *BoardId
  )
{
  UINT16        DataBuffer;

  ReadEcAdcConverter (MODEL_ID_AD, &DataBuffer);
  DEBUG ((DEBUG_INFO, "BoardId (raw) = %d\n", DataBuffer));
  // Board by max millivoltage range (of 10-bit, 3.005 V ADC)
  if (DataBuffer <= (1374 / ADC_3V_10BIT_GRANULARITY_MAX)) {
    // Consider returning an error
    DEBUG ((DEBUG_ERROR, "BoardId is reserved?\n"));
    *BoardId = 0;
  } else if (DataBuffer <= (2017 / ADC_3V_10BIT_GRANULARITY_MAX)) {
    *BoardId = BoardIdNewgateSLS_dGPU;
  } else if (DataBuffer <= (2259 / ADC_3V_10BIT_GRANULARITY_MAX)) {
    *BoardId = BoardIdRayleighSLS_960M;
  } else {
    *BoardId = BoardIdRayleighSL_dGPU;
  }
  DEBUG ((DEBUG_INFO, "BoardId = 0x%X\n", *BoardId));

  ReadEcAdcConverter (PCB_VER_AD, &DataBuffer);
  DEBUG ((DEBUG_INFO, "PCB version (raw) = %d\n", DataBuffer));
  DEBUG ((DEBUG_INFO, "PCB version: "));
  // PCB by max millivoltage range (of 10-bit, 3.005 V ADC)
  if (DataBuffer <= (2017 / ADC_3V_10BIT_GRANULARITY_MAX)) {
    // Consider returning an error
    DEBUG ((DEBUG_ERROR, "Reserved?\n"));
  } else if (DataBuffer <= (2259 / ADC_3V_10BIT_GRANULARITY_MAX)) {
    DEBUG ((DEBUG_INFO, "-1\n"));
  } else if (DataBuffer <= (2493 / ADC_3V_10BIT_GRANULARITY_MAX)) {
    DEBUG ((DEBUG_INFO, "SC\n"));
  } else if (DataBuffer <= (2759 / ADC_3V_10BIT_GRANULARITY_MAX)) {
    DEBUG ((DEBUG_INFO, "SB\n"));
  } else {
    DEBUG ((DEBUG_INFO, "SA\n"));
  }

  // FIXME
  DEBUG ((DEBUG_WARN, "OVERRIDE: Detection is unreliable and other boards unsupported!\n"));
  DEBUG ((DEBUG_INFO, "Setting board SKU to Rayleigh-SL\n"));
  *BoardId = BoardIdRayleighSL_dGPU;
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
  GetAspireVn7Dash572GBoardId (&BoardId);
  if (BoardId == BoardIdNewgateSLS_dGPU || BoardId == BoardIdRayleighSLS_960M || BoardId == BoardIdRayleighSL_dGPU) {
    LibPcdSetSku (BoardId);
    ASSERT (LibPcdGetSku() == BoardId);
  } else {
    DEBUG ((DEBUG_INFO, "BoardId not returned or valid!\n"));
    return EFI_DEVICE_ERROR;
  }

  DEBUG ((DEBUG_INFO, "SKU_ID: 0x%x\n", LibPcdGetSku()));
  return EFI_SUCCESS;
}
