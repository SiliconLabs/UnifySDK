/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef SILABS_AF_OTA_STORAGE_H
#define SILABS_AF_OTA_STORAGE_H

#define SIGNED_ENUM
#include "stack/include/ember-types.h"
#include "app/framework/include/af-types.h"

/**
 * Refer to <GSDK>/protocol/zigbee/app/framework/plugin/ota-storage-common/ota-storage.h
 * for more details.
 */

EmberAfOtaStorageStatus sli_zigbee_af_ota_storage_add_image_file(const char *filename);

#endif  // SILABS_AF_OTA_STORAGE_H