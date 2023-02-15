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
 * @defgroup unify_dotdot_attribute_store_configuration Configuration for the \ref unify_dotdot_attribute_store
 * @ingroup unify_dotdot_attribute_store
 * @brief Modules that allows to read the configuration of the
 * \ref unify_dotdot_attribute_store
 *
 * @{
 */

#ifndef UNIFY_DOTDOT_ATTRIBUTE_STORE_CONFIGURATION_H
#define UNIFY_DOTDOT_ATTRIBUTE_STORE_CONFIGURATION_H

#include "unify_dotdot_attribute_store.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Returns the registered function that allows to fetch Endpoint values
 * in the attribute store
 *
 * @returns endpoint_node_fetching_function_t
 */
endpoint_node_fetching_function_t unify_dotdot_attributes_get_endpoint_node();

/**
 * @brief Returns the registered function that allows to derive a UNID/Endpoint
 * ID from a node in the attribute store
 *
 * @returns unid_fetching_function_t
 */
unid_fetching_function_t unify_dotdot_attributes_get_unid_endpoint();

/**
 * @brief Checks if the component is configured to update the desired
 * value of DotDot attributes on command reception.
 *
 * @returns true if enabled, false if disabled
 */
bool is_desired_value_update_on_commands_enabled();

/**
 * @brief Checks if the component is configured to clear the reported values
 * of DotDot attributes when writing a new desired value.
 *
 * @returns true if enabled, false if disabled
 */
bool is_clear_reported_enabled();

/**
 * @brief Checks if the component is configured to returns deductions of supported
 * commands to DotDot MQTT on UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK calls
 *
 * @returns true if enabled, false if disabled
 */
bool is_automatic_deduction_of_supported_commands_enabled();

/**
 * @brief Checks if the component is configured to update the desired value
 * of writable ZCL attributes when receiving WriteAttributes
 *
 * @returns true if enabled, false if disabled
 */
bool is_write_attributes_enabled();

/**
 * @brief Checks if the component is configured to undefine ZCL reported attributes
 * when receiving ForceReadAttributes
 *
 * @returns true if enabled, false if disabled
 */
bool is_force_read_attributes_enabled();

/**
 * @brief Checks if the component is configured to publish ZCL attribute
 * values to MQTT
 *
 * @returns true if enabled, false if disabled
 */
bool is_publish_attribute_values_to_mqtt_enabled();

#ifdef __cplusplus
}
#endif

#endif  //UNIFY_DOTDOT_ATTRIBUTE_STORE_CONFIGURATION_H
/** @} end unify_dotdot_attribute_store_configuration */
