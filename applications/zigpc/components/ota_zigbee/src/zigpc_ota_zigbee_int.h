/*******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/**
 * @file ota_zigbee_int.h
 * @defgroup ota_zigbee_int ZigPC OTA Internal functions
 * @ingroup ota_zigbee
 * @brief This header contains the private definitions and internal 
 * functionality associated with over-the-air upgrades . 
 * All definitions and functions here are required for
 * manipulating or configuring OTA Updates
 *
 * @{
 */
#ifndef OTA_ZIGBEE_INT_H
#define OTA_ZIGBEE_INT_H

#include <string>

// Shared UIC includes
#include <ota.hpp>
#include <sl_status.h>

// ZigPC includes
#include <zcl_definitions.h>

/*
 * @brief ZIGPC_DEFAULT_OTA_PATH
 * The default path where to save OTA images
 */
static const char ZIGPC_DEFAULT_OTA_PATH[] = "./ota-files/";

/*
 * @brief ZIGPC_DEFAULT_OTA_CACHE_SIZE  
 * The default size of the cache for used to configure
 * the OTA Image Listener
 */
static const unsigned long ZIGPC_DEFAULT_OTA_CACHE_SIZE
  = 10 * 1024 * 1024;  //bytes

/*
 * @brief ZIGPC_DEFAULT_OTA_TIMEOUT  
 * The default time, in seconds for downloading an OTA image
 * from an image provider
 */
static const unsigned int ZIGPC_DEFAULT_OTA_TIMEOUT = 60 * 60;  //seconds

/**
 * @brief ota_zigbee_register_observers
 * Registers the observers for OTA events from the zigpc_gateway
 *
 * @return SL_STATUS_OK if able to successfuly register the observers
 */
sl_status_t ota_zigbee_register_observers();

/**
 * @brief on_ota_zigbee_upgrade_start
 * Callback for the start of an OTA upgrade
 */
void on_ota_zigbee_upgrade_start(void *data);

/**
 * @brief on_ota_zigbee_upgrade_complete
 * Callback for the completion of an OTA upgrade
 */
void on_ota_zigbee_upgrade_complete(void *data);

/**
 * @brief zigpc_ota_zigbee_image_available 
 * Callback for a new ota image available
 *
 * @param ota_meta_info - meta information about OTA upgrade
 */
void zigpc_ota_zigbee_image_available(uic_ota::meta_t ota_meta_info);

/**
 * @brief zigpc_ota_zigbee_image_ready 
 * Callback for a new image has been downloaded and is ready
 *
 * @param result - the status of the download
 * @param file_path - the path where the image was downloaded
 */
void zigpc_ota_zigbee_image_ready(const uic_ota::image_ready_result_t result,
                                  const std::string &file_path);

/** @} end ota_zigbee_int */
#endif  //OTA_ZIGBEE_INT_H
