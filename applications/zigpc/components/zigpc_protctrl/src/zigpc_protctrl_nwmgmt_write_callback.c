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

/* Contiki includes */
#include "sl_log.h"
#include "sl_status.h"

#include "uic_mqtt.h"

/* Boost libraries */

#include "zigpc_net_mgmt.h"

#include "zigpc_protctrl_nwmgmt_write_callback.h"
#include "zigpc_protctrl_int.h"

static const char *LOG_TAG = "zigpc_protctrl_nwmgmt_write_callback";

static const char *STATE_PARAMETER_UNID = "Unid";

extern char
  *const zigpc_protctrl_nwmgmt_state_str[ZIGPC_NET_MGMT_FSM_STATE_MAX_VAL];

sl_status_t zigpc_protctrl_nwmgmt_write_extract_known_state_parameters(
  zigpc_net_mgmt_state_change_request_t *state_change,
  const zigpc_protctrl_state_parameter_t *parameter)
{
  sl_status_t status = SL_STATUS_OK;

  if ((state_change == NULL) || (parameter == NULL)) {
    status = SL_STATUS_NULL_POINTER;
  }

  if ((status == SL_STATUS_OK) && (parameter->key_size == 0)) {
    status = SL_STATUS_EMPTY;
  }

  if (status == SL_STATUS_OK) {
    if (!strncmp(STATE_PARAMETER_UNID, parameter->key, parameter->key_size)) {
      status = str_to_zigbee_eui64(parameter->value,
                                   parameter->value_size,
                                   state_change->param_eui64);
      if (status != SL_STATUS_OK) {
        sl_log_warning(LOG_TAG,
                       "Unable to parse EUI64 from UNID: 0x%X",
                       status);
      } else {
        state_change->param_eui64_filled = true;
      }
    }
  }

  return status;
}

sl_status_t zigpc_protctrl_on_write_update_callback(
  const char *state,
  const zigpc_protctrl_state_parameter_t *state_parameter_list,
  uint8_t state_parameter_count)
{
  sl_status_t status = SL_STATUS_OK;
  zigpc_net_mgmt_state_change_request_t state_change;

  if ((state == NULL)
      || ((state_parameter_count > 0) && (state_parameter_list == NULL))) {
    status = SL_STATUS_NULL_POINTER;
    sl_log_warning(LOG_TAG,
                   "Invalid state or parameter list passsed in: 0x%X",
                   status);
  }

  if (status == SL_STATUS_OK) {
    if ((strlen(state) == 0)
        || ((state_parameter_count > 0) && (state_parameter_list == NULL))) {
      status = SL_STATUS_EMPTY;
      sl_log_warning(LOG_TAG,
                     "Empty state or parameter list passsed in: 0x%X",
                     status);
    }
  }

  state_change.param_eui64_filled = false;

  state_change.requested_state = ZIGPC_NET_MGMT_FSM_STATE_MIN_VAL;

  if (status == SL_STATUS_OK) {
    for (uint8_t i = ZIGPC_NET_MGMT_FSM_STATE_MIN_VAL + 1;
         i < ZIGPC_NET_MGMT_FSM_STATE_MAX_VAL;
         i++) {
      if ((zigpc_protctrl_nwmgmt_state_str[i] != NULL)
          && (!strncmp(zigpc_protctrl_nwmgmt_state_str[i],
                       state,
                       strlen(state)))) {
        state_change.requested_state = i;
        break;
      }
    }
    if (state_change.requested_state == ZIGPC_NET_MGMT_FSM_STATE_MIN_VAL) {
      status = SL_STATUS_INVALID_TYPE;
      sl_log_warning(LOG_TAG, "Unknown state requested: 0x%X", status);
    }
  }

  if (status == SL_STATUS_OK) {
    for (uint8_t i = 0; i < state_parameter_count; i++) {
      status = zigpc_protctrl_nwmgmt_write_extract_known_state_parameters(
        &state_change,
        &state_parameter_list[i]);
    }
  }
  if (status == SL_STATUS_OK) {
    status = zigpc_net_mgmt_state_change_request(&state_change);
    if (status != SL_STATUS_OK) {
      sl_log_warning(
        LOG_TAG,
        "Unable to request state change to '%s' in network management: 0x%X",
        state,
        status);
    }
  }
  return status;
}
