/** @file
  Logo DXE Driver, install Edkii Platform Logo protocol.

Copyright (c) 2016 - 2020, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/
#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/BmpSupportLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/BootLogo2.h>
#include <Protocol/GraphicsOutput.h>

/**
  Entrypoint of this module.

  This function is the entrypoint of this module. It prepares the BGRT
  blit-buffer.

  @param  ImageHandle       The firmware allocated handle for the EFI image.
  @param  SystemTable       A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval EFI_UNSUPPORTED   A dependency is unavailable.
  @retval EFI_NOT_FOUND     Failed to find the logo.

**/
EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
{
  EFI_STATUS                     Status;
  EDKII_BOOT_LOGO2_PROTOCOL      *BootLogo2;
  EFI_GRAPHICS_OUTPUT_PROTOCOL   *GraphicsOutput;
  UINT32                         SizeOfX;
  UINT32                         SizeOfY;
  VOID                           *BmpAddress;
  UINTN                          BmpSize;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL  *Blt;
  UINTN                          BltSize;
  UINTN                          Height;
  UINTN                          Width;
  INTN                           DestX;
  INTN                           DestY;

  //
  // MinPlatform has the FSP draw the logo.
  // Build a blit-buffer for a bitmap here and set it for a BGRT.
  //
  Status = gBS->LocateProtocol (&gEdkiiBootLogo2ProtocolGuid, NULL, (VOID **)&BootLogo2);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  // Determine BGRT display offsets
  Status = gBS->HandleProtocol (gST->ConsoleOutHandle, &gEfiGraphicsOutputProtocolGuid, (VOID **)&GraphicsOutput);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  SizeOfX = GraphicsOutput->Mode->Info->HorizontalResolution;
  SizeOfY = GraphicsOutput->Mode->Info->VerticalResolution;

  Status = GetSectionFromAnyFv (
             &gTianoLogoGuid,
             EFI_SECTION_RAW,
             0,
             &BmpAddress,
             &BmpSize
             );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  // Allocates pool for blit-buffer
  Status = TranslateBmpToGopBlt (
             BmpAddress,
             BmpSize,
             &Blt,
             &BltSize,
             &Height,
             &Width
             );
  ASSERT_EFI_ERROR (Status);

  // EdkiiPlatformLogoDisplayAttributeCenter
  DestX = (SizeOfX - Width) / 2;
  DestY = (SizeOfY - Height) / 2;

  Status = BootLogo2->SetBootLogo (BootLogo2, Blt, DestX, DestY, Width, Height);

  // SetBootLogo() allocates a copy pool, so free this.
  if (Blt != NULL) {
    FreePool (Blt);
  }

  return Status;
}
