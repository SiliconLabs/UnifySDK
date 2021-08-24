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

#include <cstring>
#include <string>
#include <vector>
#include <list>

/* Boost libraries */
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

/* UIC libraries */
#include "sl_log.h"
#include "sl_status.h"
#include "uic_mqtt.h"

#include "zigpc_common_ucl.h"
#include "zigpc_common_unid.h"

#include "zigpc_protctrl_int.h"
#include "zigpc_protctrl_process_send.h"
#include "zigpc_protctrl_nwmgmt_write.h"
#include "zigpc_protctrl_nwmgmt_write_callback.h"

namespace bpt = boost::property_tree;

static const char *LOG_TAG = "zigpc_protctrl_nwmgmt_write";

/**
 * @brief MQTT subscription handler for receiving network management
 * updates. This function will send a dynamically allocated string from the
 * character array passed in as a the MQTT message.
 *
 * @param topic           Character array containing MQTT topic triggered
 * @param message         Character array containing MQTT payload
 * @param message_length  Length of MQTT payload
 */
static void zigpc_protctrl_nwmgmt_write_on_message(const char *topic,
                                                   const char *message,
                                                   const size_t message_length)
{
  if ((message != NULL) && (message_length > 0)) {
    //must be freed!
    //will be deallocated in in zigpc_protctrl_process.c
    char *message_copy = (char *)malloc(sizeof(char) * (message_length + 1));
    memcpy(message_copy, message, message_length);
    message_copy[message_length] = '\0';

    zigpc_protctrl_process_send_event(ZIGPC_PROTCTRL_EVENT_WRITE_MESSAGE,
                                      (void *)message_copy);
  }
}

sl_status_t zigpc_protctrl_nwmgmt_write_update(char *update_message)
{
  sl_status_t status = SL_STATUS_OK;
  std::stringstream write_ss;
  bpt::ptree write_obj;
  boost::optional<std::string> state_field             = boost::none;
  boost::optional<bpt::ptree &> state_parameters_field = boost::none;
  std::vector<zigpc_protctrl_state_parameter_t> state_parameter_list;
  std::list<std::string> state_parameter_kv_list;

  if (update_message == NULL) {
    status = SL_STATUS_NULL_POINTER;
  }

  if (status == SL_STATUS_OK) {
    std::string update_message_str(update_message);

    if (update_message_str.empty()) {
      status = SL_STATUS_EMPTY;
    } else {
      write_ss << update_message_str;
    }
  }

  if (status == SL_STATUS_OK) {
    try {
      bpt::json_parser::read_json(write_ss, write_obj);
    } catch (const bpt::json_parser_error &err) {
      sl_log_warning(LOG_TAG,
                     "Failed to parse network management write data: %s",
                     err.what());
      write_obj.clear();
      status = SL_STATUS_OBJECT_READ;
    }
  }

  if ((status == SL_STATUS_OK) && (write_obj.empty())) {
    status = SL_STATUS_EMPTY;
  }

  if (status == SL_STATUS_OK) {
    state_field = write_obj.get_optional<std::string>("State");

    if (state_field == boost::none) {
      status = SL_STATUS_INVALID_CONFIGURATION;
      sl_log_warning(LOG_TAG,
                     "Required State field missing from message: 0x%X",
                     status);
    }
  }

  if (status == SL_STATUS_OK) {
    state_parameters_field = write_obj.get_child_optional("StateParameters");

    // Extract state parameters object if populated
    if ((state_parameters_field != boost::none)
        && !(*state_parameters_field).empty()) {
      // Parse each key-value pair into character arrays
      for (const bpt::ptree::value_type &it: *state_parameters_field) {
        std::size_t key_index = state_parameter_kv_list.size();
        state_parameter_kv_list.push_back(it.first);
        std::size_t val_index = state_parameter_kv_list.size();
        state_parameter_kv_list.push_back(it.second.data());

        auto key_it = std::next(state_parameter_kv_list.begin(), key_index);
        auto val_it = std::next(state_parameter_kv_list.begin(), val_index);
        zigpc_protctrl_state_parameter_t state_param = {
          .key        = (*key_it).c_str(),
          .key_size   = (*key_it).length(),
          .value      = (*val_it).c_str(),
          .value_size = (*val_it).length(),
        };
        state_parameter_list.push_back(state_param);
      }
    }
  }

  if (status == SL_STATUS_OK) {
    status
      = zigpc_protctrl_on_write_update_callback((*state_field).c_str(),
                                                state_parameter_list.data(),
                                                state_parameter_list.size());
  }

  return status;
}

sl_status_t zigpc_protctrl_nwmgmt_write_init(char *pc_unid_wo_prefix)
{
  const size_t topic_size
    = strlen(UCL_TOPIC_FMTSTR_BY_UNID_PROTCTRL_NETMGMT_WRITE)
      + strlen(ZIGPC_UNID_PREFIX) + strlen(pc_unid_wo_prefix) + 1;
  char topic_buffer[topic_size];

  // Format MQTT subscription topic string based on PC UNID and prefix.
  snprintf(topic_buffer,
           topic_size,
           UCL_TOPIC_FMTSTR_BY_UNID_PROTCTRL_NETMGMT_WRITE,
           ZIGPC_UNID_PREFIX,
           pc_unid_wo_prefix);
  uic_mqtt_subscribe(topic_buffer, zigpc_protctrl_nwmgmt_write_on_message);

  return SL_STATUS_OK;
}
