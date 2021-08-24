/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
#include <malloc.h>

/* Contiki includes */
#include "sl_log.h"
#include "sl_status.h"

#include "uic_mqtt.h"

#include "zigpc_protctrl_nwmgmt_write.h"

#include "zigpc_protctrl_int.h"

static const char LOG_TAG[] = "zigpc_protctrl_nwmgmt";

#define ZIGPC_PROTCTRL_NWMGMT_NEXT_STATES_BUFFER_SIZE  256
#define ZIGPC_PROTCTRL_NWMGMT_MQTT_TOPIC_BUFFER_SIZE   256
#define ZIGPC_PROTCTRL_NWMGMT_MQTT_PAYLOAD_BUFFER_SIZE 512

#define ZIGPC_PROTCTRL_NWMGMT_STATE_MQTT_PAYLOAD     \
  "{\"State\": \"%s\", \"SupportedStateList\": %s, " \
  "\"RequestedStateParameters\": %s}"

#define ZIGPC_PROTCTRL_NWMGMT_STATE_STR_LENGTH_MAX 32

/**
 * @brief Map between the Network Management FSM states and the
 * next states supported. This map is used by the component
 * to pass State information though MQTT publishing.
 *
 */
char *const zigpc_protctrl_nwmgmt_state_str[ZIGPC_NET_MGMT_FSM_STATE_MAX_VAL]
  = {
    [ZIGPC_NET_MGMT_FSM_STATE_IDLE]           = "idle",
    [ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD]       = "add node",
    [ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE]    = "remove node",
    [ZIGPC_NET_MGMT_FSM_STATE_NODE_INTERVIEW] = "node interview",
    [ZIGPC_NET_MGMT_FSM_STATE_INIT]           = NULL,
    [ZIGPC_NET_MGMT_FSM_STATE_MIN_VAL]        = NULL,
};

/**
 * @brief Join an array of strings with a prefix, delimeter, and posfix supplied
 * For example, if a string array of { "one", "two"} is supplied with a prefix
 * of "[START]", a delimeter of "|", and a postfix of "{POST}", the generated
 * string would be "[START]one|two{POST}". The status of this joining operation
 * is returned as a SL_STATUS_OK on success, SL_STATUS_NULL_POINTER or
 * SL_STATUS_WOULD_OVERFLOW on error.
 *
 *
 * @param str_list array of strings.
 * @param str_num string arrary items.
 * @param prefix string to insert before the array is processed.
 * @param delim delimeter to insert between array items.
 * @param postfix string to insert after the array is processed.
 * @param buffer destination character buffer to populate.
 * @param buffer_size destination character buffer size.
 * @return sl_status_t SL_STATUS_OK on success, SL_STATUS_NULL_POINTER on invalid
 * pointers passed, or SL_STATUS_WOULD_OVERFLOW if the destination buffer is
 * insufficient.
 */
sl_status_t zigpc_protctrl_str_list_join(const char *const *str_list,
                                         uint8_t str_num,
                                         char *prefix,
                                         char *delim,
                                         char *postfix,
                                         char *buffer,
                                         uint16_t buffer_size)
{
  sl_status_t status    = SL_STATUS_OK;
  char *buffer_ptr      = buffer;
  uint16_t buffer_count = 0;

  const bool is_not_valid = (str_list == NULL) || (prefix == NULL)
                            || (delim == NULL) || (postfix == NULL)
                            || (buffer == NULL);
  if (is_not_valid) {
    status = SL_STATUS_NULL_POINTER;
  }

  if (status == SL_STATUS_OK) {
    for (uint8_t i = 0; i < str_num; i++) {
      buffer_count += strlen(str_list[i]);
    }
    buffer_count += strlen(prefix) + strlen(postfix);
    buffer_count += strlen(delim) * (str_num - 1);

    if (buffer_count >= buffer_size) {
      status = SL_STATUS_WOULD_OVERFLOW;
    }
  }

  if (status == SL_STATUS_OK) {
    buffer_count = 0;
    buffer_count
      += snprintf(buffer_ptr + buffer_count, strlen(prefix) + 1, "%s", prefix);

    if (str_num > 0) {
      buffer_count += snprintf(buffer_ptr + buffer_count,
                               strlen(str_list[0]) + 1,
                               "%s",
                               str_list[0]);
    }

    for (uint8_t i = 1; i < str_num; i++) {
      buffer_count
        += snprintf(buffer_ptr + buffer_count, strlen(delim) + 1, "%s", delim);
      buffer_count += snprintf(buffer_ptr + buffer_count,
                               strlen(str_list[i]) + 1,
                               "%s",
                               str_list[i]);
    }
    snprintf(buffer_ptr + buffer_count, strlen(postfix) + 1, "%s", postfix);
  }

  return status;
}

/**
 * @brief Build a list of strings for states based on Net Mgmt FSM state list
 * passed in. This function will use an internal mapping between the Net Mgmt
 * FSM states and strings accepted by the UIC spec for states in order to
 * create a string list as a shallow copy with the string locations from the
 * internal mapping used.
 *
 * @param state_list Source FSM states
 * @param state_count FSM state count
 * @param str_list Destination String list
 * @param str_count Destination String list count
 * @return sl_status_t
 */
sl_status_t zigpc_protctrl_build_state_str_list(
  char **str_list,
  uint8_t str_count,
  const enum zigpc_net_mgmt_fsm_state *state_list,
  uint8_t state_count)
{
  sl_status_t status = SL_STATUS_OK;
  if ((state_list == NULL) || (str_list == NULL)) {
    status = SL_STATUS_NULL_POINTER;
  } else if (str_count < state_count) {
    status = SL_STATUS_WOULD_OVERFLOW;
  }

  if (status == SL_STATUS_OK) {
    for (uint8_t i = 0; i < state_count; i++) {
      (str_list[i]) = zigpc_protctrl_nwmgmt_state_str[state_list[i]];
    }
  }

  return status;
}

/**
 * Section: Process event handlers below
 */

sl_status_t zigpc_protctrl_process_event_net_init(
  const struct zigpc_net_mgmt_on_network_init *data)
{
  sl_status_t status;

  memcpy(protctrl_state.pc_eui64, data->pc_eui64, sizeof(zigbee_eui64_t));

  status = zigbee_eui64_to_str(protctrl_state.pc_eui64,
                               protctrl_state.pc_eui64_str,
                               ZIGBEE_EUI64_HEX_STR_LENGTH);

  if (status != SL_STATUS_OK) {
    /* Protocol Controller does not have a designator, exit now */
    sl_log_error(LOG_TAG, "Cannot get PC UNID, exiting...");
    process_post(PROCESS_BROADCAST, PROCESS_EVENT_EXIT, 0);
  }

  if (status == SL_STATUS_OK) {
    status = zigpc_protctrl_nwmgmt_write_init(protctrl_state.pc_eui64_str);
  }

  return status;
}

sl_status_t zigpc_protctrl_process_event_net_state_update(
  const struct zigpc_net_mgmt_on_network_state_update *data)
{
  sl_status_t status = SL_STATUS_OK;
  static char *net_mgmt_next_state_list[ZIGPC_NET_MGMT_FSM_STATE_MAX_VAL];
  static char net_update_topic[ZIGPC_PROTCTRL_NWMGMT_MQTT_TOPIC_BUFFER_SIZE];
  static char
    net_mgmt_next_state_buffer[ZIGPC_PROTCTRL_NWMGMT_NEXT_STATES_BUFFER_SIZE];
  static char net_mgmt_requested_params_buffer
    [ZIGPC_PROTCTRL_NWMGMT_NEXT_STATES_BUFFER_SIZE];
  static char
    net_update_payload[ZIGPC_PROTCTRL_NWMGMT_MQTT_PAYLOAD_BUFFER_SIZE];

  const char *found_state = zigpc_protctrl_nwmgmt_state_str[data->new_state];

  if (found_state == NULL) {
    status = SL_STATUS_NOT_FOUND;
  }

  if (status == SL_STATUS_OK) {
    status
      = zigpc_protctrl_build_state_str_list((char **)net_mgmt_next_state_list,
                                            ZIGPC_NET_MGMT_FSM_STATE_MAX_VAL,
                                            data->next_supported_states_list,
                                            data->next_supported_states_count);
    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   "Cannot build list of next states strings: 0x%X",
                   status);
    }
  }

  if (status == SL_STATUS_OK) {
    /* Get next state list formatted */
    status = zigpc_protctrl_str_list_join(
      (const char *const *)net_mgmt_next_state_list,
      data->next_supported_states_count,
      "[\"",
      "\", \"",
      "\"]",
      net_mgmt_next_state_buffer,
      ZIGPC_PROTCTRL_NWMGMT_NEXT_STATES_BUFFER_SIZE);

    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   "Cannot format next states in payload: 0x%X",
                   status);
    }
  }

  if (status == SL_STATUS_OK) {
    /* Get next requested state parmeters list formatted */
    if (data->requested_state_parameter_count > 0) {
      status = zigpc_protctrl_str_list_join(
        (const char *const *)data->requested_state_parameter_list,
        data->requested_state_parameter_count,
        "[\"",
        "\", \"",
        "\"]",
        net_mgmt_requested_params_buffer,
        ZIGPC_PROTCTRL_NWMGMT_NEXT_STATES_BUFFER_SIZE);
    } else {
      strncpy(net_mgmt_requested_params_buffer,
              "[]",
              ZIGPC_PROTCTRL_NWMGMT_NEXT_STATES_BUFFER_SIZE);
    }

    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   "Cannot format requested state params list in payload: 0x%X",
                   status);
    }
  }

  if (status == SL_STATUS_OK) {
    /* Format MQTT topic */
    int format_status = snprintf(net_update_topic,
                                 ZIGPC_PROTCTRL_NWMGMT_MQTT_TOPIC_BUFFER_SIZE,
                                 UCL_TOPIC_FMTSTR_BY_UNID_PROTCTRL_NETMGMT,
                                 ZIGPC_UNID_PREFIX,
                                 protctrl_state.pc_eui64_str);

    status = (format_status < 0) ? SL_STATUS_FAIL : SL_STATUS_OK;
    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   "Cannot format state update MQTT topic: 0x%X",
                   status);
    }
  }

  if (status == SL_STATUS_OK) {
    /* Format MQTT payload */
    int format_status = snprintf(net_update_payload,
                                 ZIGPC_PROTCTRL_NWMGMT_MQTT_PAYLOAD_BUFFER_SIZE,
                                 ZIGPC_PROTCTRL_NWMGMT_STATE_MQTT_PAYLOAD,
                                 found_state,
                                 net_mgmt_next_state_buffer,
                                 net_mgmt_requested_params_buffer);
    status            = (format_status < 0) ? SL_STATUS_FAIL : SL_STATUS_OK;
    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   "Cannot format state update MQTT payload: 0x%X",
                   status);
    }
  }
  if (status == SL_STATUS_OK) {
    uic_mqtt_publish(net_update_topic,
                     net_update_payload,
                     strlen(net_update_payload),
                     true);
  }

  return status;
}
