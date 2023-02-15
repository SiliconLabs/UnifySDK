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
 * @defgroup unify_dotdot_attribute_store Unify DotDot Attribute Store
 * @ingroup unify_components
 * @brief Component that assists storing, reading and writing DotDot Attributes
 * in the Attribute Store.
 *
 * This component helps storing, processing and publishing state of UCL/DotDot
 * attributes. It requires the application to initialize it and configure
 * an Endpoint Node location function.
 *
 * A set of opt-in configurations can also be enabled, refer to
 * @ref unify_dotdot_attribute_store_configuration_t
 *
 * @{
 */

#ifndef UNIFY_DOTDOT_ATTRIBUTE_STORE_H
#define UNIFY_DOTDOT_ATTRIBUTE_STORE_H

#include "sl_status.h"
#include "attribute_store.h"

#include "uic_typedefs.h"

/**
 * @brief Function signature that retrieves an Attribute Store node that
 * represents a UNID/Endpoint, under which ZCL attributes are located.
 */
typedef attribute_store_node_t (*endpoint_node_fetching_function_t)(
  const dotdot_unid_t unid, dotdot_endpoint_id_t endpoint_id);

/**
 * @brief Function signature that retrieves a the UNID/Endpoint values
 * associated to a given Attribute Store node
 */
typedef sl_status_t (*unid_fetching_function_t)(
  attribute_store_node_t node, char *unid, dotdot_endpoint_id_t *endpoint_id);

/**
 * @brief Configuration structure for the Unify DotDot Attribute Store component
 */
typedef struct {
  /// @brief endpoint_node_fetching_function_t function that returns the Attribute
  /// Store node for an endpoint based on a UNID/Endpoint ID.
  /// Set this to NULL if this function is not available
  endpoint_node_fetching_function_t get_endpoint_node_function;
  /// @brief unid_fetching_function_t function that outputs UNID/Endpoints
  /// based on a Attribute Store node. The attribute Store node may be located
  /// at any depth under the endpoint node.
  unid_fetching_function_t get_unid_endpoint_function;
  /// @brief Enables or disables the automatic updating of ZCL attribute desired
  /// values upon MQTT command reception
  bool update_attribute_desired_values_on_commands;
  /// @brief Instructs if the component should clear reported values when
  /// setting a new desired values. It can e.g. used in combination with the
  /// Attribute Resolver to force reading the state of an attribute after issuing a command
  /// Note that this has no effect if @ref update_attribute_desired_values_on_commands is set to false.
  bool clear_reported_on_desired_updates;
  /// @brief Enables or disables the automatic deduction of supported commands
  /// for a node based on the ZCL attributes present in the attribute store.
  /// When DotDot MQTT performs a UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK
  /// call to its command registered callbacks, this component can be configured
  /// to return SL_STATUS_OK based on the list of supported attributes.
  bool automatic_deduction_of_supported_commands;
  /// @brief When enabled, ForceReadAttribute will undefine the reported
  /// value of ZCL/DotDot attribute to trigger a re-resolution
  bool force_read_attributes_enabled;
  /// @brief Enables or disables the automatic publishing of ZCL Attributes
  /// values to MQTT
  bool write_attributes_enabled;
  /// @brief Enables or disables the automatic publishing of ZCL Attributes
  /// values to MQTT
  bool publish_attribute_values_to_mqtt;
} unify_dotdot_attribute_store_configuration_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the Unify Dotdot Attribute Store component.
 *
 * @return SL_STATUS_OK on success, any other value in case of error
 */
sl_status_t unify_dotdot_attribute_store_init();

/**
 * @brief Configures the Unify Dotdot Attribute Store component.
 *
 * @param configuration   Pointer to a configuration struct. Refer to
 *                        unify_dotdot_attribute_store_configuration_t.
 */
void unify_dotdot_attribute_store_set_configuration(
  const unify_dotdot_attribute_store_configuration_t *configuration);

#ifdef __cplusplus
}
#endif

#endif  //UNIFY_DOTDOT_ATTRIBUTE_STORE_H
/** @} end unify_dotdot_attribute_store */
