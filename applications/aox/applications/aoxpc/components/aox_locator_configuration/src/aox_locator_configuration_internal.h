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

/**
 * @defgroup aox_locator_configuration_internal AoXLocator Configuration Internals
 * @ingroup aox_locator_configuration
 * @brief AoX Locator configuration internal functions
 *
 * Funtions shared within the component.
 *
 * @{
 */

#ifndef AOX_LOCATOR_CONFIGURATION_INTERNAL_H
#define AOX_LOCATOR_CONFIGURATION_INTERNAL_H

#include "dotdot_mqtt.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief WriteAttribute callback for AoXLocator cluster
 *
 * This function is registered to DotDot MQTT to handle incoming
 * WriteAttribute messages for the AoXLocator cluster
 *
 * @param unid                  UNID for the destination of the command
 * @param endpoint              Endpoint ID for the destination of the command
 * @param call_type             Type of call (support verification or "normal")
 * @param state                 List of attributes and their value
 * @param updated_state         List of attributes that were updated
 * @return sl_status_t
 */
sl_status_t aox_locator_write_attributes_callback(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uic_mqtt_dotdot_aox_locator_state_t state,
  uic_mqtt_dotdot_aox_locator_updated_state_t updated_state);

/**
 * @brief Resets the AoX Locator attributes to 0.
 *
 * @return SL_STATUS_OK if successful, any other code otherwise.
 */
sl_status_t aox_locator_reset_configuration();

/**
 * @brief Invokes the registered callback for AoXLocator attribute updates
 *
 */
void aox_locator_notify_configuration_was_updated();

#ifdef __cplusplus
}
#endif

#endif  //AOX_LOCATOR_CONFIGURATION_INTERNAL_H
/** @} end aox_locator_configuration_internal */
