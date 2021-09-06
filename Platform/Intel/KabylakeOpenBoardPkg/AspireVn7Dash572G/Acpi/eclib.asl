/** @file
  This file contains the EC library for ACPI.

  Copyright (c) 2021, Baruch Binyamin Doron
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <EcCommands.h>

Mutex (EMTX, 0)

OperationRegion (ECO1, SystemIO, EC_D_PORT, 1)
Field (ECO1, ByteAcc, Lock, Preserve)
{
  ECDT, 8
}

OperationRegion (ECO2, SystemIO, EC_C_PORT, 1)
Field (ECO2, ByteAcc, Lock, Preserve)
{
  ECSC, 8
}

// Send EC command
// Return time elapsed on success, EC_TIME_OUT on failure
Method (SECC, 1, Serialized)
{
  // Wait EC timeout for input buffer to be empty
  Local0 = 0
  While ((ECSC & EC_S_IBF) && Local0 < EC_TIME_OUT)
  {
    Stall (1)
    Local0++
  }
  // Send command if timeout was not reached
  If (Local0 < EC_TIME_OUT)
  {
    ECSC = Arg0
  }

  // Return status
  Return (Local0)
}

// Send EC data
// Return time elapsed on success, EC_TIME_OUT on failure
Method (SECD, 1, Serialized)
{
  // Wait EC timeout for input buffer to be empty
  Local0 = 0
  While ((ECSC & EC_S_IBF) && Local0 < EC_TIME_OUT)
  {
    Stall (1)
    Local0++
  }
  // Send data if timeout was not reached
  If (Local0 < EC_TIME_OUT)
  {
    ECDT = Arg0
  }

  // Return status
  Return (Local0)
}

// Receive EC data
// Return data on success, EC_TIME_OUT on failure
Method (RECD, 0, Serialized)
{
  // Wait EC timeout for input buffer to be empty
  Local0 = 0
  While ((ECSC & EC_S_OBF) && Local0 < EC_TIME_OUT)
  {
    Stall (1)
    Local0++
  }
  // Return data
  If (Local0 < EC_TIME_OUT)
  {
    Return (ECDT)
  }

  // Timeout exceeded, return failure
  Return (Local0)
}

// Read EC byte
// Return data on success, EC_TIME_OUT on failure
Method (RBEC, 1, Serialized)
{
  // Check for mutex acquired to not run with another function
  Local0 = Acquire (EMTX, 0xFFFF)
  If (Local0 == 0)
  {
    Local0 = SECC (EC_C_ACPI_READ)
  }

  If (Local0 < EC_TIME_OUT)
  {
    // Send address
    Local0 = SECD (Arg0)
  }

  If (Local0 < EC_TIME_OUT)
  {
    // Receive data
    Local0 = RECD ()
  }

  Release (EMTX)
  Return (Local0)
}

// Write EC byte
// Return time elapsed on success, EC_TIME_OUT on failure
Method (WBEC, 2, Serialized)
{
  // Check for mutex acquired to not run with another function
  Local0 = Acquire (EMTX, 0xFFFF)
  If (Local0 == 0)
  {
    Local0 = SECC (EC_C_ACPI_READ)
  }

  If (Local0 < EC_TIME_OUT)
  {
    // Send address
    Local0 = SECD (Arg0)
  }

  If (Local0 < EC_TIME_OUT)
  {
    // Send data
    Local0 = SECD (Arg1)
  }

  Release (EMTX)
  // Return data
  Return (Local0)
}
