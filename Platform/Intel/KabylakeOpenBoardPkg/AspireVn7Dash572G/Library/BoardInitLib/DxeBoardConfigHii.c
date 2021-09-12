/** @file
  Installs Aspire VN7-572G board config and handles the HII callbacks.
  NOTE: Variable structure is expected to change, so in-place updates are fragile.
  - An updated structure may be larger than a present variable. Will this over-read,
    or will HII validation mitigate this?

  Copyright (c) 2021, Baruch Binyamin Doron
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "DxeBoardInitLib.h"
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/HiiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <BoardConfigNvData.h>

BOARD_CONFIG_CALLBACK_DATA  gBoardConfigPrivate = {
  BOARD_CONFIG_CALLBACK_DATA_SIGNATURE,
  NULL,
  NULL,
  {
    BoardConfigExtractConfig,
    BoardConfigRouteConfig,
    BoardConfigCallback
  }
};

EFI_GUID  mBoardConfigFormsetGuid = BOARD_CONFIG_FORMSET_GUID;

HII_VENDOR_DEVICE_PATH  mBoardConfigHiiVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    BOARD_CONFIG_FORMSET_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};

/**
  This function allows a caller to extract the current configuration for one
  or more named elements from the target driver.


  @param This            Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param Request         A null-terminated Unicode string in <ConfigRequest> format.
  @param Progress        On return, points to a character in the Request string.
                         Points to the string's null terminator if request was successful.
                         Points to the most recent '&' before the first failing name/value
                         pair (or the beginning of the string if the failure is in the
                         first name/value pair) if the request was not successful.
  @param Results         A null-terminated Unicode string in <ConfigAltResp> format which
                         has all values filled in for the names in the Request string.
                         String to be allocated by the called function.

  @retval  EFI_SUCCESS            The Results is filled with the requested values.
  @retval  EFI_OUT_OF_RESOURCES   Not enough memory to store the results.
  @retval  EFI_INVALID_PARAMETER  Request is illegal syntax, or unknown name.
  @retval  EFI_NOT_FOUND          Routing data doesn't match any storage in this driver.

**/
EFI_STATUS
EFIAPI
BoardConfigExtractConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
{
  EFI_STATUS           Status;
  UINTN                DataSize;
  BOARD_CONFIGURATION  BoardConfig;

  if (Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *Progress = Request;
  if ((Request != NULL) &&
    !HiiIsConfigHdrMatch (Request, &mBoardConfigFormsetGuid, BOARD_CONFIG_NV_NAME)) {
    return EFI_NOT_FOUND;
  }

  // Get variable
  DataSize = sizeof (BoardConfig);
  Status = gRT->GetVariable (
                  BOARD_CONFIG_NV_NAME,
                  &mBoardConfigFormsetGuid,
                  NULL,
                  &DataSize,
                  &BoardConfig
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Use HII helper to convert variable data to config
  Status = gHiiConfigRouting->BlockToConfig (
                                gHiiConfigRouting,
                                Request,
                                (VOID *) &BoardConfig,
                                DataSize,
                                Results,
                                Progress
                                );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a(): Failed to retrieve board config - %r!\n", Status));
  }

  return Status;
}

/**
  This function processes the results of changes in configuration.


  @param This            Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param Configuration   A null-terminated Unicode string in <ConfigResp> format.
  @param Progress        A pointer to a string filled in with the offset of the most
                         recent '&' before the first failing name/value pair (or the
                         beginning of the string if the failure is in the first
                         name/value pair) or the terminating NULL if all was successful.

  @retval  EFI_SUCCESS            The Results is processed successfully.
  @retval  EFI_INVALID_PARAMETER  Configuration is NULL.
  @retval  EFI_NOT_FOUND          Routing data doesn't match any storage in this driver.

**/
EFI_STATUS
EFIAPI
BoardConfigRouteConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Configuration,
  OUT EFI_STRING                             *Progress
  )
{
  EFI_STATUS           Status;
  UINTN                DataSize;
  BOARD_CONFIGURATION  BoardConfig;

  if (Configuration == NULL || Progress == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *Progress  = Configuration;
  if (!HiiIsConfigHdrMatch (Configuration, &mBoardConfigFormsetGuid, BOARD_CONFIG_NV_NAME)) {
    return EFI_NOT_FOUND;
  }

  // Get variable
  DataSize = sizeof (BoardConfig);
  Status = gRT->GetVariable (
                  BOARD_CONFIG_NV_NAME,
                  &mBoardConfigFormsetGuid,
                  NULL,
                  &DataSize,
                  &BoardConfig
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Use HII helper to convert updated config to variable data
  Status = gHiiConfigRouting->ConfigToBlock (
                                gHiiConfigRouting,
                                Configuration,
                                (VOID *) &BoardConfig,
                                &DataSize,
                                Progress
                                );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a(): Failed to convert board config - %r!\n", Status));
  }

  // Set variable
  Status = gRT->SetVariable (
                  BOARD_CONFIG_NV_NAME,
                  &mBoardConfigFormsetGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                  DataSize,
                  &BoardConfig
                  );

  return Status;
}

/**
  This callback function is registered with the formset. When user selects a configuration,
  this call back function will be triggered.


  @param This            Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param Action          Specifies the type of action taken by the browser.
  @param QuestionId      A unique value which is sent to the original exporting driver
                         so that it can identify the type of data to expect.
  @param Type            The type of value for the question.
  @param Value           A pointer to the data being sent to the original exporting driver.
  @param ActionRequest   On return, points to the action requested by the callback function.

  @retval  EFI_SUCCESS           The callback successfully handled the action.
  @retval  EFI_INVALID_PARAMETER The setup browser call this function with invalid parameters.

**/
EFI_STATUS
EFIAPI
BoardConfigCallback (
  IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL      *This,
  IN     EFI_BROWSER_ACTION                    Action,
  IN     EFI_QUESTION_ID                       QuestionId,
  IN     UINT8                                 Type,
  IN     EFI_IFR_TYPE_VALUE                    *Value,
     OUT EFI_BROWSER_ACTION_REQUEST            *ActionRequest
  )
{
  if ((Value == NULL) || (ActionRequest == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if (Action != EFI_BROWSER_ACTION_CHANGED) {
    return EFI_UNSUPPORTED;
  }

  if (QuestionId == QUESTION_SAVE_EXIT) {
    *ActionRequest = EFI_BROWSER_ACTION_REQUEST_FORM_SUBMIT_EXIT;
  } else if (QuestionId == QUESTION_DISCARD_EXIT) {
    *ActionRequest = EFI_BROWSER_ACTION_REQUEST_FORM_DISCARD_EXIT;
  }

  return EFI_SUCCESS;
}

/**
  This function installs the HII form.

**/
VOID
EFIAPI
InstallBoardConfigHiiForm (
  VOID
  )
{
  EFI_STATUS           Status;
  BOARD_CONFIGURATION  BoardConfig;
  EFI_STRING           ConfigRequestHdr;
  UINTN                DataSize;
  BOOLEAN              ActionFlag;

  DEBUG ((DEBUG_INFO, "%a() Starts\n", __FUNCTION__));

  //
  // Install Device Path and Config Access protocols to driver handle
  //
  gBoardConfigPrivate.DriverHandle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &gBoardConfigPrivate.DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mBoardConfigHiiVendorDevicePath,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &gBoardConfigPrivate.ConfigAccess,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Publish our HII data
  //
  gBoardConfigPrivate.HiiHandle = HiiAddPackages (
                                    &mBoardConfigFormsetGuid,
                                    gBoardConfigPrivate.DriverHandle,
                                    BoardConfigVfrBin,
                                    DxeBoardInitLibStrings,
                                    NULL
                                    );
  ASSERT (gBoardConfigPrivate.HiiHandle != NULL);

  //
  // Initialise VarStore data.
  //
  ZeroMem (&BoardConfig, sizeof (BoardConfig));
  ConfigRequestHdr = HiiConstructConfigHdr (
                       &mBoardConfigFormsetGuid,
                       BOARD_CONFIG_NV_NAME,
                       gBoardConfigPrivate.DriverHandle
                       );
  ASSERT (ConfigRequestHdr != NULL);

  // Attempt to retrieve variable
  DataSize = sizeof (BoardConfig);
  Status = gRT->GetVariable (
                  BOARD_CONFIG_NV_NAME,
                  &mBoardConfigFormsetGuid,
                  NULL,
                  &DataSize,
                  &BoardConfig
                  );
  // HII helper functions will use ExtractConfig() and RouteConfig(),
  // where we will set the variable as required
  if (!EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Config variable exists, validate contents\n"));
    ActionFlag = HiiValidateSettings (ConfigRequestHdr);
    if (!ActionFlag) {
      DEBUG ((DEBUG_INFO, "Variable is invalid, reset to defaults\n"));
      ActionFlag = HiiSetToDefaults (ConfigRequestHdr, EFI_HII_DEFAULT_CLASS_STANDARD);
      ASSERT (ActionFlag);
    }
  } else {
    DEBUG ((DEBUG_INFO, "Config variable does not exist, create and set to defaults\n"));
    Status = gRT->SetVariable (
                    BOARD_CONFIG_NV_NAME,
                    &mBoardConfigFormsetGuid,
                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                    DataSize,
                    &BoardConfig
                    );
    ASSERT_EFI_ERROR (Status);
    ActionFlag = HiiSetToDefaults (ConfigRequestHdr, EFI_HII_DEFAULT_CLASS_STANDARD);
    ASSERT (ActionFlag);
  }

  FreePool (ConfigRequestHdr);

  DEBUG ((DEBUG_INFO, "%a() Ends\n", __FUNCTION__));
}

/**
  This function uninstalls the HII form.

**/
VOID
EFIAPI
UninstallBoardConfigHiiForm (
  VOID
  )
{
  EFI_STATUS           Status;

  DEBUG ((DEBUG_INFO, "%a() Starts\n", __FUNCTION__));

  //
  // Uninstall Device Path and Config Access protocols
  //
  Status = gBS->UninstallMultipleProtocolInterfaces (
                  gBoardConfigPrivate.DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mBoardConfigHiiVendorDevicePath,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &gBoardConfigPrivate.ConfigAccess,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Remove our HII data
  //
  HiiRemovePackages (gBoardConfigPrivate.HiiHandle);

  DEBUG ((DEBUG_INFO, "%a() Ends\n", __FUNCTION__));
}
