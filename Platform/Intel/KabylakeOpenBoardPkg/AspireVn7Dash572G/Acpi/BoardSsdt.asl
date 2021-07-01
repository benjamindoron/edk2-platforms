/** @file
  This file contains the Aspire VN7-572G SSDT Table ASL code.

Copyright (c) 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

DefinitionBlock (
  "Board.aml",
  "SSDT",
  0x02,
  "Board ",
  "Board  ",
  0x20141018
  )
{
  Include ("ec.asl")
  Include ("mainboard.asl")
}
