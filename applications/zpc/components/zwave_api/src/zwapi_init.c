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
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "zwapi_init.h"
#include "zwapi_func_ids.h"
#include "zwapi_protocol_basis.h"
#include "zwapi_session.h"
#include "zwapi_internal.h"
#include "sl_log.h"

#define LOG_TAG "zwapi_init"

static zwapi_chip_data_t chip;

const char *zw_lib_names[] = {
  "Unknown",
  "Static Controller Node",
  "Bridge Controller Node library",
  "Portable Controller Node library",
  "Enhanced End Node library",
  "End Node library",
  "Installer library",
};

// Module shared variables
zwapi_callbacks_t *callbacks;
bool awaiting_zwave_api_started;

static void zwapi_init_local_variables()
{
  memset(&chip, 0, sizeof(chip));
  awaiting_zwave_api_started = false;
}

bool zwapi_support_command_func(uint8_t func_id)
{
  if ((func_id == FUNC_ID_SERIAL_API_GET_CAPABILITIES)
      || (chip.supported_bitmask[(func_id - 1) >> 3]
          & (1 << ((func_id - 1) & 0x7)))) {
    return true;
  } else {
    return false;
  }
}

bool zwapi_support_setup_func(serial_api_setup_cmd_t setup_cmd)
{
  uint8_t cmd = setup_cmd;
  if (chip.supported_setup_bitmask & cmd) {
    return true;
  } else {
    return false;
  }
}

sl_status_t zwapi_init(const char *serial_port,
                       int *serial_fd,
                       const zwapi_callbacks_t *_callbacks)
{
  int file_descriptor = zwapi_session_init(serial_port);
  if (file_descriptor <= 0) {
    return SL_STATUS_FAIL;
  }
  if (serial_fd != NULL) {
    *serial_fd = file_descriptor;
  }

  zwapi_init_local_variables();

  zwave_api_protocol_init_callbacks();
  if (_callbacks != NULL) {
    callbacks = (zwapi_callbacks_t *)_callbacks;
  }

  return zwapi_refresh_capabilities();
}

void zwapi_destroy(void)
{
  zwapi_session_shutdown();
}

bool zwapi_poll()
{
  uint8_t *frame = NULL;
  uint8_t len    = 0;

  zwapi_session_enqueue_rx_frames();
  bool more_frames = zwapi_session_dequeue_frame(&frame, &len);

  if (frame) {
    zwave_api_protocol_rx_dispatch(frame, len);
    free(frame);
  }

  return more_frames;
}

sl_status_t zwapi_refresh_capabilities(void)
{
  // Get the Z-Wave module capabilities
  uint8_t response_buffer[FRAME_LENGTH_MAX] = {0};
  uint8_t response_length                   = 0;
  sl_status_t capabilities_status = zwapi_session_send_frame_with_response(
    FUNC_ID_SERIAL_API_GET_CAPABILITIES,
    NULL,
    0,
    response_buffer,
    &response_length);

  if (capabilities_status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Failed to fetch capabilities from the Z-Wave module\n");
    assert(false);
    return capabilities_status;
  }
  if (response_length > (IDX_DATA + 7)) {
    uint8_t current_index = IDX_DATA;
    chip.version_major    = response_buffer[current_index++];
    chip.version_minor    = response_buffer[current_index++];
    chip.manufacturer_id  = response_buffer[current_index++] << 8;
    chip.manufacturer_id |= response_buffer[current_index++];
    chip.product_type = response_buffer[current_index++] << 8;
    chip.product_type |= response_buffer[current_index++];
    chip.product_id = response_buffer[current_index++] << 8;
    chip.product_id |= response_buffer[current_index++];
    memcpy(chip.supported_bitmask,
           &(response_buffer[current_index]),
           response_length - (current_index - 1));
  } else {
    return SL_STATUS_FAIL;
  }

  if (zwapi_support_command_func(FUNC_ID_SERIAL_API_SETUP)) {
    response_length                             = 0;
    uint8_t index                               = 0;
    uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0};
    request_buffer[index++] = SERIAL_API_SETUP_CMD_SUPPORTED;

    sl_status_t setup_status
      = zwapi_session_send_frame_with_response(FUNC_ID_SERIAL_API_SETUP,
                                               request_buffer,
                                               index,
                                               response_buffer,
                                               &response_length);
    if (setup_status != SL_STATUS_OK) {
      sl_log_error(
        LOG_TAG,
        "Failed to fetch supported setup commands from the Z-Wave module\n");
      assert(false);
      return setup_status;
    }

    if (response_length > (IDX_DATA + 1)
        && response_buffer[IDX_DATA] == ZW_COMMAND_RETURN_VALUE_TRUE) {
      chip.supported_setup_bitmask = response_buffer[IDX_DATA + 1];
    } else {
      return SL_STATUS_FAIL;
    }
  }

  chip.library_type = zwapi_get_zw_library_version(response_buffer);
  if (chip.library_type < sizeof(zw_lib_names) / sizeof(char *)) {
    sl_log_info(LOG_TAG, "Z-Wave library version : %s\n", &response_buffer[7]);
    sl_log_info(LOG_TAG,
                "Z-Wave library type    : %s\n",
                zw_lib_names[chip.library_type]);
  }

  // Update our local cache of the chip type/version.
  sl_status_t init_data_status = zwapi_get_init_data(NULL,
                                                     &chip.capability_flag,
                                                     NULL,
                                                     chip.node_list,
                                                     &chip.chip_type,
                                                     &chip.chip_revision);

  if (init_data_status != SL_STATUS_OK) {
    return init_data_status;
  }

  // Get the current frequency, if known by the module
  chip.rf_region = zwapi_get_rf_region();

  return SL_STATUS_OK;
}

sl_status_t zwapi_get_init_data(uint8_t *ver,
                                uint8_t *capabilities,
                                uint8_t *len,
                                zwave_nodemask_t node_list,
                                uint8_t *chip_type,
                                uint8_t *chip_version)
{
  uint8_t response_length = 0, node_list_length = 0;
  uint8_t response_buffer[FRAME_LENGTH_MAX] = {0};
  sl_status_t send_command_status
    = zwapi_session_send_frame_with_response(FUNC_ID_SERIAL_API_GET_INIT_DATA,
                                             NULL,
                                             0,
                                             response_buffer,
                                             &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA) {
    uint8_t current_index = IDX_DATA;
    // FIXME: Ideally we verify that current_index never become
    // greater or equal to response_length all the way,
    // else we will copy garbage into the pointers.
    if (ver) {
      *ver = response_buffer[current_index];
    }
    current_index++;

    if (capabilities) {
      *capabilities = response_buffer[current_index];
    }
    current_index++;

    if (len) {
      *len = response_buffer[current_index];
    }
    node_list_length = response_buffer[current_index];
    current_index++;

    for (uint8_t i = 0; i < node_list_length; i++) {
      if (node_list) {
        node_list[i] = response_buffer[current_index];
      }
      current_index++;
    }
    if (chip_type) {
      *chip_type = response_buffer[current_index];
    }
    current_index++;

    if (chip_version) {
      *chip_version = response_buffer[current_index];
    }

    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

sl_status_t zwapi_get_node_list(zwave_nodemask_t node_list)
{
  // Refresh our local copy of the node list first
  sl_status_t command_status
    = zwapi_get_init_data(NULL, NULL, NULL, chip.node_list, NULL, NULL);

  if (command_status == SL_STATUS_OK) {
    if (node_list != NULL) {
      memcpy(node_list, chip.node_list, sizeof(chip.node_list));
    }
  } else {
    sl_log_info(LOG_TAG, "Failed to retrieve NodeID list from the Z-Wave API");
  }
  return command_status;
}

void zwapi_get_app_version(uint8_t *major, uint8_t *minor)
{
  *major = chip.version_major;
  *minor = chip.version_minor;
}

void zwapi_get_chip_type_version(uint8_t *type, uint8_t *version)
{
  *type    = chip.chip_type;
  *version = chip.chip_revision;
}

void zwapi_get_chip_data(zwapi_chip_data_t *user_chip_data)
{
  memcpy(user_chip_data, &chip, sizeof(chip));
}

zwapi_library_type_t zwapi_get_library_type()
{
  return chip.library_type;
}

sl_status_t
  zwapi_get_protocol_version(zwapi_protocol_version_information_t *protocol)
{
  uint8_t response_length                   = 0;
  uint8_t response_buffer[FRAME_LENGTH_MAX] = {0};
  sl_status_t send_command_status
    = zwapi_session_send_frame_with_response(FUNC_ID_ZW_GET_PROTOCOL_VERSION,
                                             NULL,
                                             0,
                                             response_buffer,
                                             &response_length);

  if (send_command_status == SL_STATUS_OK) {
    if ((response_length - IDX_DATA) >= 1) {
      protocol->type = response_buffer[IDX_DATA];
    }
    if ((response_length - IDX_DATA) >= 4) {
      protocol->revision_version = response_buffer[IDX_DATA + 3];
      protocol->minor_version    = response_buffer[IDX_DATA + 2];
      protocol->major_version    = response_buffer[IDX_DATA + 1];
    }

    if ((response_length - IDX_DATA) >= 6) {
      protocol->build_number = response_buffer[IDX_DATA + 5];
      protocol->build_number += response_buffer[IDX_DATA + 4] << 8;
    }
    if ((response_length - IDX_DATA) >= (6 + sizeof(protocol->git_commit))) {
      memcpy(&protocol->git_commit,
             &response_buffer[IDX_DATA + 6],
             sizeof(protocol->git_commit));
    }
    return SL_STATUS_OK;
  }
  return SL_STATUS_FAIL;
}

// CLEANUP: Moved from sdk_versioning.c
const char *
  zwapi_generate_sdk_version_from_protocol_version(const char *version_string)
{
  const char *sdk_version = "0.00.00";  // Unknown version

  if (version_string) {
    int major = 0;
    int minor = 0;

    // The version string is encoded as "Z-Wave M.mm"
    int args_processed
      = sscanf(version_string, "Z-Wave %1d.%2d", &major, &minor);
    if (2 == args_processed) {
      /* Mapping major protocol version to SDK versions
      * See Z-Wave protocol versions table in:
      * "Z-Wave 700 Series Appl. Programmers Guide" (INS14258)
      */
      switch (major * 100 + minor) {
        case 412:
        case 414:
          sdk_version = "6.60.00";
          break;
        case 433:
          sdk_version = "6.61.00";
          break;
        case 462:
          sdk_version = "6.61.01";
          break;
        case 428:
          sdk_version = "6.70.00";
          break;
        case 445:
          sdk_version = "6.70.01";
          break;
        case 460:
          sdk_version = "6.71.00";
          break;
        case 461:
          sdk_version = "6.71.01";
          break;
        case 502:
          sdk_version = "6.71.02";
          break;
        case 503:
          sdk_version = "6.71.03";
          break;
        case 601:
          sdk_version = "6.81.00";
          break;
        case 602:
          sdk_version = "6.81.01";
          break;
        case 603:
          sdk_version = "6.81.02";
          break;
        case 604:
          sdk_version = "6.81.03";
          break;
        case 605:
          sdk_version = "6.81.04";
          break;
        case 606:
          sdk_version = "6.81.05";
          break;  // Unconfirmed
        case 700:
          sdk_version = "7.00.00";
          break;
        case 711:
          sdk_version = "7.11.00";
          break;
        case 712:
          sdk_version = "7.12.00";
          break;
        default:
          /* Ideally the above table should be updated with every new release.
          * But in case this does not always happen we make a simple generic
          * fallback mapping from protocol version to sdk version
          */
          if (6 == major) {
            sdk_version = "6.8x.xx";
            break;
          }
          if (7 == major) {
            sdk_version = "7.xx.xx";
            break;
          }
          if (8 == major) {
            sdk_version = "8.xx.xx";
            break;
          }
          break;
      }
    }
  }
  return sdk_version;
}

const char *
  zwapi_generate_nvm_id_from_sdk_version(const char *sdk_ver,
                                         zwapi_library_type_t library_type)
{
  static char nvm_id_buf[20] = {};

  nvm_id_buf[0] = '\0';

  if (sdk_ver) {
    /* The SDK version is assumed as M.mm.pp, i.e. first digit is major
     * version number (see zwapi_generate_sdk_version_from_protocol_version())
     */
    if (strncmp("7.11.00", sdk_ver, 7) == 0) {
      return "bridge7.11";
    } else if (sdk_ver[0] > '6') {
      /* For now we don't differentiate between the versions following
       * version 6, we simply assume they all use the nvm3 file system
       * driver.
       */
      return "bridge7.12";
    } else {
      const char *library_type_str = "";

      switch (library_type) {
        case ZWAVE_LIBRARY_TYPE_CONTROLLER_BRIDGE:
          library_type_str = "bridge";
          break;
        case ZWAVE_LIBRARY_TYPE_CONTROLLER_STATIC:
          library_type_str = "static";
          break;
        case ZWAVE_LIBRARY_TYPE_CONTROLLER:
          library_type_str = "portable";
          break;
        default:
          library_type_str = "ERROR";
          break;
      }

      /* As the NVM layout for SDK 6.8x was only changed after 6.81.00, we handle 6.81.00 as a 6.7 controller. */
      if (strncmp(sdk_ver, "6.81.00", 7) == 0) {
        snprintf(nvm_id_buf, sizeof(nvm_id_buf), "%s6.7", library_type_str);
      } else {
        /* Only use the first three chars of the sdk version,
        * i.e. 6.81.03 becomes 6.8. End result e.g. "bridge6.8"
        */
        snprintf(nvm_id_buf,
                 sizeof(nvm_id_buf),
                 "%s%.3s",
                 library_type_str,
                 sdk_ver);
      }
    }
  }
  return nvm_id_buf;  // Static buffer
}

sl_status_t zwapi_send_command(uint8_t command,
                               const uint8_t *payload_buffer,
                               uint8_t payload_buffer_length)
{
  if (!zwapi_support_command_func(command)) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  return zwapi_session_send_frame(command,
                                  payload_buffer,
                                  payload_buffer_length);
}

sl_status_t zwapi_send_command_with_response(uint8_t command,
                                             const uint8_t *payload_buffer,
                                             uint8_t payload_buffer_length,
                                             uint8_t *response_buffer,
                                             uint8_t *response_length)
{
  if (!zwapi_support_command_func(command)) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  return zwapi_session_send_frame_with_response(command,
                                                payload_buffer,
                                                payload_buffer_length,
                                                response_buffer,
                                                response_length);
}

sl_status_t zwapi_send_command_no_ack(uint8_t command,
                                      const uint8_t *payload_buffer,
                                      uint8_t payload_buffer_length)
{
  if (!zwapi_support_command_func(command)) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  return zwapi_session_send_frame_no_ack(command,
                                         payload_buffer,
                                         payload_buffer_length);
}

bool zwapi_is_awaiting_zwave_api_started(){
  return awaiting_zwave_api_started;
}

zwave_lr_channel_t zwapi_get_zwave_lr_channel()
{
  uint8_t response_buffer[FRAME_LENGTH_MAX] = {0};
  uint8_t response_length                   = 0;
  sl_status_t command_status
    = zwapi_session_send_frame_with_response(FUNC_ID_GET_LR_CHANNEL,
                                             NULL,
                                             0,
                                             response_buffer,
                                             &response_length);
  if (command_status == SL_STATUS_OK && response_length > IDX_DATA) {
    return response_buffer[IDX_DATA];
  }
  return ZWAVE_LONG_RANGE_CHANNEL_NOT_SUPPORTED;
}

sl_status_t zwapi_set_zwave_lr_channel(zwave_lr_channel_t new_channel)
{
  uint8_t request_buffer[] = {new_channel};
  return zwapi_send_command(FUNC_ID_SET_LR_CHANNEL,
                            request_buffer,
                            sizeof(request_buffer));
}
