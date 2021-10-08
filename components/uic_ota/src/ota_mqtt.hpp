/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

/**
 * @file ota_mqtt.hpp
 * @brief OTA status updates module 
 *
 * This module implements the logic for publishing OTA subscriptions.
 *
 * @{
 */

#ifndef OTA_MQTT_HPP
#define OTA_MQTT_HPP

// Standard component
#include <string>

// This component
#include "ota.hpp"

// UIC Components
#include "uic_typedefs.h"
#include "sl_status.h"

/**
 * @brief Publish OTA status update for string value. 
 * 
 * @param unid unique identifier id.
 * @param endpoint endpoint id.
 * @param uiid unique image identifier id.
 * @param val string value.
 * @param status the status topic to update to.
 * @param cluster_revision cluster revision number.
 */
void publish_status_string(const dotdot_unid_t &unid,
                           const dotdot_endpoint_id_t &endpoint,
                           const ota_uiid_t &uiid,
                           const std::string &val,
                           const std::string &status,
                           uint32_t cluster_revision);

/**
 * @brief Publish OTA status update for unsigned long value. 
 * 
 * @param unid unique identifier id.
 * @param endpoint endpoint id.
 * @param uiid unique image identifier id.
 * @param val unsgined long value.
 * @param status the status topic to update to.
 * @param cluster_revision cluster revision number.
 */
void publish_status_unsigned_long(const dotdot_unid_t &unid,
                                  const dotdot_endpoint_id_t &endpoint,
                                  const ota_uiid_t &uiid,
                                  const unsigned long val,
                                  const std::string &status,
                                  uint32_t cluster_revision);

/**
 * @brief Convert last error to label. 
 * 
 * @param value last error enum value.
 * 
 * @returns label of enum value.
 */
std::string convert_last_error_to_label(uic_ota::last_error_t last_error);

/**
 * @brief Conver status to label. 
 * 
 * @param value last error enum value.
 * 
 * @returns label of enum value.
 */
std::string convert_status_to_label(uic_ota::status_t status);

#endif  //OTA_MQTT_HPP
/** @} end ota_mqtt */
