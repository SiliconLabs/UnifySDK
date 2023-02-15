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
#include "unify_dotdot_attribute_store.h"
#include "unify_dotdot_attribute_store_configuration.h"

////////////////////////////////////////////////////////////////////////////////
// Private functions
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Default function that will be invoked as a No-Op if the application
 * has not provided a function to locate endpoints in the Attribute Store.
 *
 * @param unid             UNID of the node to located in the Attribute Store
 * @param endpoint_id      Endpoint ID to locate in the Attribute Store
 * @returns ATTRIBUTE_STORE_INVALID_NODE, always
 */
static attribute_store_node_t
  get_default_endpoint_node(const dotdot_unid_t unid,
                            dotdot_endpoint_id_t endpoint_id)
{
  return ATTRIBUTE_STORE_INVALID_NODE;
}

/**
 * @brief Default function that will be invoked as a No-Op if the application
 * has not provided a function to resolve UNID/Endpoints ID from nodes in
 * the Attribute Store.
 *
 * @param node             Refer to @ref unid_fetching_function_t
 * @param unid             Refer to @ref unid_fetching_function_t
 * @param endpoint_id      Refer to @ref unid_fetching_function_t
 * @returns SL_STATUS_FAIL, always
 */
static sl_status_t get_default_unid_endpoint(attribute_store_node_t node,
                                             char *unid,
                                             dotdot_endpoint_id_t *endpoint_id)
{
  unid[0]      = '\0';
  *endpoint_id = 0;
  return SL_STATUS_FAIL;
}

////////////////////////////////////////////////////////////////////////////////
// Constant and static variables
////////////////////////////////////////////////////////////////////////////////
// Constant default configuration:
static const unify_dotdot_attribute_store_configuration_t DEFAULT_CONFIGURATION
  = {.get_endpoint_node_function                  = &get_default_endpoint_node,
     .get_unid_endpoint_function                  = &get_default_unid_endpoint,
     .update_attribute_desired_values_on_commands = false,
     .clear_reported_on_desired_updates           = false,
     .automatic_deduction_of_supported_commands   = false,
     .force_read_attributes_enabled               = false,
     .write_attributes_enabled                    = false,
     .publish_attribute_values_to_mqtt            = false};

// Static variables
static const unify_dotdot_attribute_store_configuration_t *configuration
  = &DEFAULT_CONFIGURATION;

////////////////////////////////////////////////////////////////////////////////
// Functions internal to the components
////////////////////////////////////////////////////////////////////////////////
endpoint_node_fetching_function_t unify_dotdot_attributes_get_endpoint_node()
{
  if (configuration->get_endpoint_node_function != NULL) {
    return configuration->get_endpoint_node_function;
  } else {
    return &get_default_endpoint_node;
  }
}

unid_fetching_function_t unify_dotdot_attributes_get_unid_endpoint()
{
  if (configuration->get_unid_endpoint_function != NULL) {
    return configuration->get_unid_endpoint_function;
  } else {
    return &get_default_unid_endpoint;
  }
}

bool is_desired_value_update_on_commands_enabled()
{
  return configuration->update_attribute_desired_values_on_commands;
}

bool is_clear_reported_enabled()
{
  return configuration->clear_reported_on_desired_updates;
}

bool is_automatic_deduction_of_supported_commands_enabled()
{
  return configuration->automatic_deduction_of_supported_commands;
}

bool is_write_attributes_enabled()
{
  return configuration->write_attributes_enabled;
}

bool is_force_read_attributes_enabled()
{
  return configuration->force_read_attributes_enabled;
}

bool is_publish_attribute_values_to_mqtt_enabled()
{
  return configuration->publish_attribute_values_to_mqtt;
}

////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////
void unify_dotdot_attribute_store_set_configuration(
  const unify_dotdot_attribute_store_configuration_t *_configuration)
{
  if (_configuration == NULL) {
    configuration = &DEFAULT_CONFIGURATION;
  } else {
    configuration = _configuration;
  }
}