/** @file
  Header file for NV data structure definition.

Copyright (c) 2015 - 2018, Intel Corporation. All rights reserved.<BR>
Copyright (c) 2021, Baruch Binyamin Doron
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __BOARD_CONFIG_NV_DATA_H__
#define __BOARD_CONFIG_NV_DATA_H__

#define BOARD_CONFIG_FORMSET_GUID \
  { \
    0x6E38A4A7, 0xB6B7, 0x41E0, { 0xA6, 0xF3, 0x41, 0x35, 0x72, 0xDF, 0x88, 0x2F } \
  }

#define BOARD_CONFIGURATION_VARSTORE_ID  0x0001
#define BOARD_CONFIGURATION_FORM_ID      0x0001

#define BOARD_LOCK_DOWN_BIOS_LOCK         0x2000
#define BOARD_LOCK_DOWN_PMC_READ_DISABLE  0x2001

#define QUESTION_SAVE_EXIT     0x2ffe
#define QUESTION_DISCARD_EXIT  0x2fff

//
// NV data structure
//
typedef struct {
  UINT8   LockDownBiosLock;
  UINT8   LockDownPmcReadDisable;
} BOARD_CONFIGURATION;

#define BOARD_CONFIG_NV_NAME  L"BoardSetup"

#endif
