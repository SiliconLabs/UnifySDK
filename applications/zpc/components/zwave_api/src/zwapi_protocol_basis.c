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

#include <string.h>
#include "zwapi_protocol_basis.h"
#include "zwapi_func_ids.h"
#include "zwapi_internal.h"
#include "zwapi_session.h"
#include "zwapi_init.h"
#include "sl_log.h"
#include "zwapi_utils.h"

#define LOG_TAG "zwapi_protocol_basis"

/**
 * @brief Struct that helps us cache the RF Region and avoid
 *        querying the Z-Wave API all the time for something that
 *        does not change.
 *
 */
typedef struct zwave_rf_region_cache {
  // Indicates if the cached RF region is valid or should be queried again.
  bool valid;
  // Indicates which RF region was reported by the Z-Wave API.
  zwave_rf_region_t rf_region;
} zwave_rf_region_cache_t;

// Static variables
static zwave_node_id_basetype_t node_id_basetype_setting = NODEID_8BITS;
static zwave_rf_region_cache_t rf_region_cache
  = {.valid = false, .rf_region = ZWAVE_RF_REGION_UNDEFINED};

sl_status_t zwapi_soft_reset(void)
{
  rf_region_cache.valid     = false;
  rf_region_cache.rf_region = ZWAVE_RF_REGION_UNDEFINED;
  return zwapi_send_command(FUNC_ID_SERIAL_API_SOFT_RESET, NULL, 0);
}

uint8_t zwapi_get_zw_protocol_status(void)
{
  uint8_t response_length                   = 0;
  uint8_t response_buffer[FRAME_LENGTH_MAX] = {0};
  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_GET_PROTOCOL_STATUS,
                                       NULL,
                                       0,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA) {
    return response_buffer[IDX_DATA];
  } else {
    return 0;  //CLEANUP: What to return as protocol status when we did not get a response?
  }
}

uint8_t zwapi_get_zw_library_type(void)
{
  uint8_t response_length                   = 0;
  uint8_t response_buffer[FRAME_LENGTH_MAX] = {0};
  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_TYPE_LIBRARY,
                                       NULL,
                                       0,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA) {
    return response_buffer[IDX_DATA];
  } else {
    // Return 0 as it is not a valid Z-Wave library
    return 0;
  }
}

uint8_t zwapi_get_zw_library_version(uint8_t *pBuf)
{
  uint8_t response_length                   = 0;
  uint8_t response_buffer[FRAME_LENGTH_MAX] = {0};
  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_GET_VERSION,
                                       NULL,
                                       0,
                                       response_buffer,
                                       &response_length);
  if (send_command_status == SL_STATUS_OK
      && response_length > (IDX_DATA + 12)) {
    if (pBuf) {
      memcpy(pBuf, (void *)&response_buffer, response_length);
    }
    return response_buffer[IDX_DATA + 12];
  }
  // Return 0 as it is not a valid Z-Wave library
  return 0;
}

sl_status_t zwapi_set_rf_receive_mode(uint8_t mode)
{
  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  request_buffer[index++]                     = mode;
  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_SET_RF_RECEIVE_MODE,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);
  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA
      && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

/*RET The powerlevel set */
rf_power_level_t zwapi_set_rf_power_level(
  rf_power_level_t powerLevel) /* IN Powerlevel to set */
{
  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  request_buffer[index++]                     = (uint8_t)powerLevel;
  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_RF_POWER_LEVEL_SET,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);
  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA) {
    return response_buffer[IDX_DATA];
  } else {
    return NORMAL_POWER;
  }
}

/*RET The current powerlevel */
rf_power_level_t zwapi_get_rf_power_level(void)
{
  uint8_t response_length                   = 0;
  uint8_t response_buffer[FRAME_LENGTH_MAX] = {0};
  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_RF_POWER_LEVEL_GET,
                                       NULL,
                                       0,
                                       response_buffer,
                                       &response_length);
  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA) {
    return response_buffer[IDX_DATA];
  } else {
    return NORMAL_POWER;
  }
}

sl_status_t zwapi_set_tx_status_reporting(bool enable)
{
  if (!zwapi_support_command_func(FUNC_ID_SERIAL_API_SETUP)
      || !zwapi_support_setup_func(SERIAL_API_SETUP_CMD_TX_STATUS_REPORT)) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  uint8_t response_length                   = 0;
  uint8_t response_buffer[FRAME_LENGTH_MAX] = {0};
  uint8_t request_buffer[] = {0, SERIAL_API_SETUP_CMD_TX_STATUS_REPORT, enable};

  sl_status_t frame_status
    = zwapi_session_send_frame_with_response(FUNC_ID_SERIAL_API_SETUP,
                                             request_buffer,
                                             sizeof(request_buffer),
                                             response_buffer,
                                             &response_length);
  if (frame_status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG,
                   "Failed to configure tx status report for send_data\n");
  }

  return frame_status;
}

sl_status_t zwapi_set_tx_power_level(tx_power_level_t txpowerlevel)
{
  if (!zwapi_support_command_func(FUNC_ID_SERIAL_API_SETUP)
      || !zwapi_support_setup_func(SERIAL_API_SETUP_CMD_TX_POWERLEVEL_SET)) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  request_buffer[index++] = SERIAL_API_SETUP_CMD_TX_POWERLEVEL_SET;
  request_buffer[index++] = txpowerlevel.normal;
  request_buffer[index++] = txpowerlevel.measured0dBm;
  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_SERIAL_API_SETUP,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > (IDX_DATA + 1)
      && response_buffer[IDX_DATA] == SERIAL_API_SETUP_CMD_TX_POWERLEVEL_SET
      && response_buffer[IDX_DATA + 1] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    return SL_STATUS_OK;
  }
  return SL_STATUS_FAIL;
}

tx_power_level_t zwapi_get_tx_power_level(void)
{
  tx_power_level_t txpowerlevel = {.normal = 0, .measured0dBm = 0};

  if (zwapi_support_command_func(FUNC_ID_SERIAL_API_SETUP)
      && zwapi_support_setup_func(SERIAL_API_SETUP_CMD_TX_POWERLEVEL_GET)) {
    uint8_t response_length = 0, index = 0;
    uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
            response_buffer[FRAME_LENGTH_MAX]   = {0};

    request_buffer[index++] = SERIAL_API_SETUP_CMD_TX_POWERLEVEL_GET;
    sl_status_t send_command_status
      = zwapi_send_command_with_response(FUNC_ID_SERIAL_API_SETUP,
                                         request_buffer,
                                         index,
                                         response_buffer,
                                         &response_length);

    if (send_command_status == SL_STATUS_OK) {
      if (response_length > (IDX_DATA + 1)) {
        txpowerlevel.normal = response_buffer[IDX_DATA + 1];
      }
      if (response_length > (IDX_DATA + 2)) {
        txpowerlevel.measured0dBm = response_buffer[IDX_DATA + 2];
      }
    }
  }
  return txpowerlevel;
}

sl_status_t zwapi_set_rf_region(zwave_rf_region_t rfregion)
{
  if (!zwapi_support_command_func(FUNC_ID_SERIAL_API_SETUP)
      || !zwapi_support_setup_func(SERIAL_API_SETUP_CMD_RF_REGION_SET)) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // If we try to change the RF Region, mark the cached value as invalid
  rf_region_cache.valid     = false;
  rf_region_cache.rf_region = ZWAVE_RF_REGION_UNDEFINED;

  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};

  request_buffer[index++] = SERIAL_API_SETUP_CMD_RF_REGION_SET;
  request_buffer[index++] = (uint8_t)rfregion;
  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_SERIAL_API_SETUP,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > (IDX_DATA + 1)
      && response_buffer[IDX_DATA] == SERIAL_API_SETUP_CMD_RF_REGION_SET
      && response_buffer[IDX_DATA + 1] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

zwave_rf_region_t zwapi_get_rf_region(void)
{
  // Use the previously cached value, if available.
  if (rf_region_cache.valid == true) {
    return rf_region_cache.rf_region;
  }

  if (!zwapi_support_setup_func(SERIAL_API_SETUP_CMD_RF_REGION_GET)) {
    return ZWAVE_RF_REGION_UNDEFINED;
  }

  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};

  request_buffer[index++] = SERIAL_API_SETUP_CMD_RF_REGION_GET;
  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_SERIAL_API_SETUP,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > (IDX_DATA + 1)) {
    rf_region_cache.valid     = true;
    rf_region_cache.rf_region = response_buffer[IDX_DATA + 1];
    return rf_region_cache.rf_region;
  }

  return ZWAVE_RF_REGION_UNDEFINED;
}

sl_status_t zwapi_get_maximum_payload_size(uint8_t *maximum_size)
{
  if (!zwapi_support_command_func(FUNC_ID_SERIAL_API_SETUP)
      || !zwapi_support_setup_func(
        SERIAL_API_SETUP_CMD_MAXIMUM_PAYLOAD_SIZE_GET)) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};

  request_buffer[index++] = SERIAL_API_SETUP_CMD_MAXIMUM_PAYLOAD_SIZE_GET;
  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_SERIAL_API_SETUP,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > (IDX_DATA + 1)
      && response_buffer[IDX_DATA]
           == SERIAL_API_SETUP_CMD_MAXIMUM_PAYLOAD_SIZE_GET) {
    *maximum_size = response_buffer[IDX_DATA + 1];
    return SL_STATUS_OK;
  }
  return SL_STATUS_FAIL;
}

sl_status_t zwapi_get_lr_maximum_payload_size(uint8_t *maximum_size)
{
  if (!zwapi_support_command_func(FUNC_ID_SERIAL_API_SETUP)
      || !zwapi_support_setup_func(
        SERIAL_API_SETUP_CMD_LR_MAXIMUM_PAYLOAD_SIZE_GET)) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};

  request_buffer[index++] = SERIAL_API_SETUP_CMD_LR_MAXIMUM_PAYLOAD_SIZE_GET;
  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_SERIAL_API_SETUP,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > (IDX_DATA + 1)
      && response_buffer[IDX_DATA]
           == SERIAL_API_SETUP_CMD_LR_MAXIMUM_PAYLOAD_SIZE_GET) {
    *maximum_size = response_buffer[IDX_DATA + 1];
    return SL_STATUS_OK;
  }
  return SL_STATUS_FAIL;
}

sl_status_t zwapi_set_node_id_basetype(zwave_node_id_basetype_t basetype)
{
  if (!zwapi_supports_long_range()) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};

  request_buffer[index++] = SERIAL_API_SETUP_CMD_NODEID_BASETYPE_SET;
  request_buffer[index++] = basetype;
  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_SERIAL_API_SETUP,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > (IDX_DATA + 1)
      && response_buffer[IDX_DATA] == SERIAL_API_SETUP_CMD_NODEID_BASETYPE_SET
      && response_buffer[IDX_DATA + 1] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    node_id_basetype_setting = basetype;
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

zwave_node_id_basetype_t zwapi_get_node_id_basetype()
{
  return node_id_basetype_setting;
}

sl_status_t zwapi_send_node_information(zwave_node_id_t destNode,
                                        uint8_t txOptions,
                                        void (*completedFunc)(uint8_t))
{
  uint8_t func_id, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0};
  func_id = (completedFunc == NULL ? 0 : 0x03);
  zwapi_write_node_id(request_buffer, &index, destNode);
  request_buffer[index++]              = txOptions;
  request_buffer[index++]              = func_id;  // Func id for CompletedFunc
  zwapi_send_node_information_callback = completedFunc;
  return zwapi_send_command(FUNC_ID_ZW_SEND_NODE_INFORMATION,
                            request_buffer,
                            index);
}

sl_status_t zwapi_send_test_frame(zwave_node_id_t destination_node_id,
                                  rf_power_level_t power_level,
                                  void (*callback_function)(uint8_t tx_status))
{
  uint8_t func_id, response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};

  func_id                        = (callback_function == NULL ? 0 : 4);
  zwapi_send_test_frame_callback = callback_function;
  zwapi_write_node_id(request_buffer, &index, destination_node_id);
  request_buffer[index++] = (uint8_t)power_level;
  request_buffer[index++] = func_id;
  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_SEND_TEST_FRAME,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);
  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA
      && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

sl_status_t zwapi_get_random_word(uint8_t *random_buffer,
                                  uint8_t number_of_random_bytes)
{
  // Maximum 32 random bytes can be returned by the Z-Wave module
  if (number_of_random_bytes > 32) {
    number_of_random_bytes = 32;
  }

  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};

  request_buffer[index++] = number_of_random_bytes;
  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_GET_RANDOM,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);
  if (send_command_status == SL_STATUS_OK && response_length > (IDX_DATA + 1)
      && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    for (uint8_t i = 0; i < response_buffer[IDX_DATA + 1]; i++) {
      if (i >= number_of_random_bytes || response_length < (IDX_DATA + i + 2)) {
        break;
      }
      random_buffer[i] = response_buffer[IDX_DATA + i + 2];
    }
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

sl_status_t zwapi_get_random_byte(uint8_t *user_byte)
{
  uint8_t response_length                   = 0;
  uint8_t response_buffer[FRAME_LENGTH_MAX] = {0};

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_RANDOM,
                                       NULL,
                                       0,
                                       response_buffer,
                                       &response_length);
  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA) {
    *user_byte = response_buffer[IDX_DATA];
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

/* TODO - Unify zwapi_explore_request_inclusion() and setlearnMode - Let protocol handle */
/* the inclusion - zwapi_explore_request_inclusion() should internally call SetLearnMode not let App do it */
sl_status_t zwapi_explore_request_inclusion()
{
  uint8_t response_length                   = 0;
  uint8_t response_buffer[FRAME_LENGTH_MAX] = {0};

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_EXPLORE_REQUEST_INCLUSION,
                                       NULL,
                                       0,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA
      && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

/* TODO - Unify zwapi_explore_request_exclusion() and setlearnMode - Let protocol handle */
/* the exclusion - zwapi_explore_request_exclusion() should internally call SetLearnMode not let App do it */
sl_status_t zwapi_explore_request_exclusion()
{
  uint8_t response_length                   = 0;
  uint8_t response_buffer[FRAME_LENGTH_MAX] = {0};

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_EXPLORE_REQUEST_EXCLUSION,
                                       NULL,
                                       0,
                                       response_buffer,
                                       &response_length);
  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA
      && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

sl_status_t zwapi_disable_watchdog()
{
  return zwapi_send_command(FUNC_ID_ZW_WATCHDOG_DISABLE, NULL, 0);
}

sl_status_t zwapi_enable_watchdog()
{
  return zwapi_send_command(FUNC_ID_ZW_WATCHDOG_ENABLE, NULL, 0);
}

sl_status_t zwapi_start_watchdog()
{
  return zwapi_send_command(FUNC_ID_ZW_WATCHDOG_START, NULL, 0);
}

sl_status_t zwapi_get_background_rssi(uint8_t *rssi_values,
                                      uint8_t *values_length)
{
  uint8_t response_buffer[FRAME_LENGTH_MAX] = {0};
  uint8_t number_of_channels                = 0;

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_ZW_GET_BACKGROUND_RSSI,
                                       NULL,
                                       0,
                                       response_buffer,
                                       &number_of_channels);

  if (send_command_status == SL_STATUS_OK) {
    // Total reply length is IDX_DATA more than number of channels
    number_of_channels = number_of_channels - IDX_DATA;
    if (number_of_channels > 3) {
      number_of_channels = 3;
      // sl_log_warning(LOG_TAG,  "zwapi_get_background_rssi: Too much data returned\n");
    }
    *values_length = number_of_channels;
    memcpy(rssi_values, &response_buffer[IDX_DATA], number_of_channels);
    return SL_STATUS_OK;
  }

  *values_length = 0;
  return SL_STATUS_FAIL;
}

sl_status_t zwapi_set_application_node_information(uint8_t listening,
                                                   node_type_t node_type,
                                                   const uint8_t *nodeParm,
                                                   uint8_t parmLength)
{
  uint8_t index                               = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0};
  request_buffer[index++]                     = listening;
  request_buffer[index++]                     = node_type.generic;
  request_buffer[index++]                     = node_type.specific;
  request_buffer[index++]                     = parmLength;

  for (uint8_t i = 0; i != parmLength; i++) {
    request_buffer[index++] = nodeParm[i];
  }
  return zwapi_send_command(FUNC_ID_SERIAL_API_APPL_NODE_INFORMATION,
                            request_buffer,
                            index);
}

sl_status_t zwapi_enable_auto_program_mode(void)
{
  /* Chip will reboot after receiving this command, so send as raw bytes without
   * waiting for SerialAPI ACKs */
  return zwapi_send_command_no_ack(FUNC_ID_AUTO_PROGRAMMING, NULL, 0);
}

sl_status_t zwapi_get_full_node_list(zwave_nodemask_t node_list)
{
  uint16_t length = 0;

  // First the regular list, then the extended.
  sl_status_t node_list_status = zwapi_get_node_list(node_list);
  sl_status_t extended_node_list_status
    = zwapi_get_long_range_nodes(&length, node_list);

  // if 1 of them returns OK, then we return OK
  // since there are some nodes added to the mask.
  if (node_list_status == SL_STATUS_OK
      || extended_node_list_status == SL_STATUS_OK) {
    return SL_STATUS_OK;
  }
  return SL_STATUS_FAIL;
}

sl_status_t zwapi_get_long_range_nodes(uint16_t *number_of_bytes,
                                       zwave_nodemask_t lr_node_list)
{
  uint8_t more_nodes = 1, start_offset = 0;

  /* Seek to the point where LR nodes start in zwave_nodemask_t */
  lr_node_list += ZW_LR_NODEMASK_OFFSET;
  sl_status_t command_status = SL_STATUS_OK;
  *number_of_bytes           = 0;

  while (more_nodes == 1) {
    uint8_t response_length = 0, index = 0;
    uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
            response_buffer[FRAME_LENGTH_MAX]   = {0};
    uint8_t received_nodelist_length            = 0;
    request_buffer[index++]                     = start_offset;

    command_status
      = zwapi_send_command_with_response(FUNC_ID_SERIAL_API_GET_LR_NODES,
                                         request_buffer,
                                         index,
                                         response_buffer,
                                         &response_length);

    if (command_status == SL_STATUS_OK) {
      more_nodes               = response_buffer[IDX_DATA];
      start_offset             = response_buffer[IDX_DATA + 1];
      received_nodelist_length = response_buffer[IDX_DATA + 2];
      *number_of_bytes += received_nodelist_length;
      uint8_t *p = &response_buffer[IDX_DATA + 3];
      for (int i = 0; i < received_nodelist_length; i++) {
        if ((i + (start_offset * 128)) > sizeof(zwave_nodemask_t)) {
          sl_log_warning(LOG_TAG,
                         "Z-Wave API get LR node list index of bound\n");
          return SL_STATUS_FAIL;
        }
        lr_node_list[i + start_offset * 128] = *p++;
      }
      start_offset++;
    } else {
      sl_log_info(
        LOG_TAG,
        "Failed to retrieve extended NodeID list from the Z-Wave API");
      return command_status;
    }
  }
  return command_status;
}

sl_status_t zwapi_get_long_range_channel(uint8_t *lr_channel)
{
  uint8_t response_length                   = 0;
  uint8_t response_buffer[FRAME_LENGTH_MAX] = {0};
  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_GET_LR_CHANNEL,
                                       NULL,
                                       0,
                                       response_buffer,
                                       &response_length);
  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA) {
    *lr_channel = response_buffer[IDX_DATA];
  }
  return send_command_status;
}

sl_status_t zwapi_set_long_range_channel(uint8_t lr_channel)
{
  uint8_t response_length = 0, index = 0;
  uint8_t response_buffer[FRAME_LENGTH_MAX]   = {0},
          request_buffer[REQUEST_BUFFER_SIZE] = {0};
  request_buffer[index++]                     = lr_channel;
  return zwapi_send_command_with_response(FUNC_ID_SET_LR_CHANNEL,
                                          request_buffer,
                                          index,
                                          response_buffer,
                                          &response_length);
}

bool zwapi_supports_long_range()
{
  /* This becomes not true if long range is supported on a module where only
   * low node IDs is supported.
   * If that ever happens, the solution may be using the bit in FUNC_ID_SERIAL_API_STARTED
   * to indicate whether module supports; however, FUNC_ID_SERIAL_API_STARTED only exist in
   * the response of soft_reset(). We may do a reset at start to get the global view of this.
   */
  if (!zwapi_support_command_func(FUNC_ID_SERIAL_API_SETUP)
      || !zwapi_support_setup_func(SERIAL_API_SETUP_CMD_NODEID_BASETYPE_SET)) {
    return false;
  }
  return true;
}
