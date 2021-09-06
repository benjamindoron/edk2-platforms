/** @file
  This file contains the Aspire VN7-572G SSDT Table ASL code.

  Copyright (c) 2021, Baruch Binyamin Doron
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <IndustryStandard/Acpi63.h>

DefinitionBlock (
  "Board.aml",
  "SSDT",
  EFI_ACPI_6_3_DIFFERENTIATED_SYSTEM_DESCRIPTION_TABLE_REVISION,
  "ACRSKL",
  "AcerSKL ",
  0x20141018
  )
{
  External (SSMP, IntObj)
  External (SMIF, IntObj)
  External (\MDBG, MethodObj)

  // SW SMI data port
  OperationRegion (DPRT, SystemIO, 0xB3, 1)
  Field (DPRT, ByteAcc, Lock, Preserve)
  {
    SSDP, 8
  }

  Name (ESMI, 0xDD)  // TODO: Patch at runtime
  Method (TRPS, 3, Serialized)
  {
    \DBGH (Concatenate ("SMIF: ", ToHexString (Arg0)))
    \DBGH (Concatenate ("Param0: ", ToHexString (Arg1)))
    \DBGH (Concatenate ("Param1: ", ToHexString (Arg2)))

    Local0 = Arg1
    Local0 |= (Arg2 << 4)
    \DBGH (Concatenate ("Local0: ", ToHexString (Local0)))

    SMIF = Arg0
    SSDP = Local0
    SSMP = ESMI
  }

  // Debug print helper
  Method (DBGH, 1)
  {
    // If present, print to ACPI debug feature's buffer
    If (CondRefOf (\MDBG))
    {
      \MDBG (Arg0)
    }
    // Always use "Debug" object for operating system
    Debug = Arg0
  }

  Include ("ec.asl")
  Include ("mainboard.asl")
}
