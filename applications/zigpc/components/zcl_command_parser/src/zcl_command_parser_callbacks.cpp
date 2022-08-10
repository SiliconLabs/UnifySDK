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

/* Shared Unify includes */
#include "sl_status.h"
#include "sl_log.h"
#include "multi_invoke.hpp"

/* ZigPC includes */
#include "zigpc_common_zigbee.h"
#include "zigpc_gateway_notify.h"

#include "zcl_command_parser.h"
#include "zcl_command_parser_int.h"

constexpr char LOG_TAG[] = "zcl_command_parser_callbacks";

/**
 * @brief stores callbacks when commands are parsed
 */
static multi_invoke<uint32_t,
                    const zigbee_eui64_t,
                    const zigbee_endpoint_id_t,
                    const zigpc_zclcmdparse_callback_data_t *>
  command_listeners;

uint32_t zigpc_zclcmdparse_to_key(zcl_cluster_id_t cluster_id,
                                  zcl_command_id_t command_id)
{
  return (cluster_id << 16) | (command_id);
}

void zigpc_zclcmdparse_reset_listeners(void)
{
  command_listeners.clear();
}

void zigpc_zclcmdparse_invoke_callback(
  zigbee_eui64_t eui64,
  zigbee_endpoint_id_t endpoint_id,
  zcl_cluster_id_t cluster_id,
  zcl_command_id_t command_id,
  const zigpc_zclcmdparse_callback_data_t *data)
{
  bool is_supported
    = zigpc_zclcmdparse_cluster_command_supported(cluster_id, command_id);

  if (is_supported == true) {
    command_listeners(zigpc_zclcmdparse_to_key(cluster_id, command_id),
                      eui64,
                      endpoint_id,
                      data);
  }
}

sl_status_t
  zigpc_zclcmdparse_register_callback(zcl_cluster_id_t cluster_id,
                                      zcl_command_id_t command_id,
                                      zigpc_zclcmdparse_callback_t callback)
{
  sl_status_t status = SL_STATUS_OK;

  bool is_supported
    = zigpc_zclcmdparse_cluster_command_supported(cluster_id, command_id);

  if (is_supported == false) {
    status = SL_STATUS_INVALID_TYPE;
  } else {
    command_listeners.add(zigpc_zclcmdparse_to_key(cluster_id, command_id),
                          callback);
  }
  return status;
}

sl_status_t
  zigpc_zclcmdparse_remove_callback(zcl_cluster_id_t cluster_id,
                                    zcl_command_id_t command_id,
                                    zigpc_zclcmdparse_callback_t callback)
{
  sl_status_t status = SL_STATUS_OK;

  bool is_supported
    = zigpc_zclcmdparse_cluster_command_supported(cluster_id, command_id);

  if (is_supported == false) {
    status = SL_STATUS_INVALID_TYPE;
  } else {
    command_listeners.remove(zigpc_zclcmdparse_to_key(cluster_id, command_id),
                             callback);
  }
  return status;
}

sl_status_t zigpc_zclcmdparse_setup(void)
{
  sl_status_t status
    = zigpc_gateway_register_observer(ZIGPC_GATEWAY_NOTIFY_ZCL_COMMAND_RECEIVED,
                                      zigpc_zclcmdparse_on_command_received);

  sl_log_debug(LOG_TAG, "Setup status: 0x%X", status);

  return status;
}
