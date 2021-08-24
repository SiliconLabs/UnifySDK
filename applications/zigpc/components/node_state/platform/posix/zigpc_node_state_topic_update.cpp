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

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "sl_status.h"
#include "sl_log.h"
#include "uic_mqtt.h"

#include "zigpc_common_ucl.h"
#include "zigpc_common_unid.h"

#include "zigpc_node_state_int.h"

namespace bpt = boost::property_tree;

static const char *LOG_TAG = "zigpc_node_state_topic_update";

static const char
  *zigpc_node_state_node_status_strings[ZIGBEE_NODE_STATUS_MAX_VAL]
  = {
    [ZIGBEE_NODE_STATUS_INVALID]         = NULL,
    [ZIGBEE_NODE_STATUS_NODEID_ASSIGNED] = "Online interviewing",
    [ZIGBEE_NODE_STATUS_INCLUDED]        = "Online functional",
    [ZIGBEE_NODE_STATUS_INTERVIEW_FAIL]  = "Online non-functional",
    [ZIGBEE_NODE_STATUS_INTERVIEWING]    = "Online interviewing",
    [ZIGBEE_NODE_STATUS_OFFLINE]         = "Offline",
    [ZIGBEE_NODE_STATUS_UNAVAILABLE]     = "Unavailable",
};

sl_status_t zigpc_node_state_process_event_node_status_change(
  const zigpc_node_state_status_change_t *state_change)
{
  sl_status_t status       = SL_STATUS_OK;
  bool topic_buffer_filled = false;
  char *topic_buffer;
  char eui64_str[ZIGBEE_EUI64_HEX_STR_LENGTH];

  if ((state_change->status == ZIGBEE_NODE_STATUS_INVALID)
      || (state_change->status >= ZIGBEE_NODE_STATUS_MAX_VAL)) {
    status = SL_STATUS_INVALID_TYPE;
  }

  if (status == SL_STATUS_OK) {
    status = zigbee_eui64_to_str(state_change->eui64,
                                 eui64_str,
                                 ZIGBEE_EUI64_HEX_STR_LENGTH);
    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG, "Formatting EUI64 to string failed: 0x%X", status);
    }
  }

  if (status == SL_STATUS_OK) {
    const size_t topic_size = strlen(UCL_TOPIC_FMTSTR_BY_UNID_STATE)
                              + strlen(ZIGPC_UNID_PREFIX) + strlen(eui64_str)
                              + 1;
    topic_buffer = new char[topic_size];
    // Format MQTT subscription topic string based on PC UNID and prefix.
    int format_status = snprintf(topic_buffer,
                                 topic_size,
                                 UCL_TOPIC_FMTSTR_BY_UNID_STATE,
                                 ZIGPC_UNID_PREFIX,
                                 eui64_str);

    if (format_status < 0) {
      delete[] topic_buffer;
      status = SL_STATUS_FAIL;
      sl_log_error(LOG_TAG,
                   "Cannot format state update MQTT payload: 0x%X",
                   status);
    } else {
      topic_buffer_filled = true;
    }
  }

  if (status == SL_STATUS_OK) {
    bpt::ptree payload;
    payload.put("Security", "Zigbee Z3.0");
    payload.put("MaximumCommandDelay", 1);
    payload.put("NetworkStatus",
                zigpc_node_state_node_status_strings[state_change->status]);
    std::stringstream ss;
    bpt::write_json(ss, payload);
    uic_mqtt_publish(topic_buffer, ss.str().data(), ss.str().size(), true);
  }

  if (topic_buffer_filled == true) {
    delete[] topic_buffer;
  }

  return status;
}