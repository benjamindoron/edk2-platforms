/** @file
  EC library functions and definitions.

  This library provides basic EC interface.

  There may be different libraries for different environments (PEI, BS, RT, SMM).
  Make sure you meet the requirements for the library (protocol dependencies, use
  restrictions, etc).

Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _BOARD_EC_LIB_H_
#define _BOARD_EC_LIB_H_

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
  );

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
  );

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
  );

#endif
