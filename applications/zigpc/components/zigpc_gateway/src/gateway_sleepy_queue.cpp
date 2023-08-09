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
#include <queue>
#include <unordered_map>

#include <algorithm>
#include <functional>
#include <string>
#include <cstring>

#include "gateway_sleepy_queue.h"

//quality of life typedefs
typedef std::queue<gateway_frame_t> sleepy_message_list_t;

//use the string rep of an eui64 as the key
std::unordered_map<std::string, sleepy_message_list_t> sleepy_device_map;

sl_status_t zigpc_gateway_store_sleepy_frame(gateway_frame_t frame)
{
  sl_status_t status = SL_STATUS_OK;

  char eui64_cstr[ZIGBEE_EUI64_HEX_STR_LENGTH];
  status = zigbee_eui64_to_str(frame.eui64, eui64_cstr, ZIGBEE_EUI64_HEX_STR_LENGTH);
  std::string eui64_str(eui64_cstr);
  if (status == SL_STATUS_OK){
  sleepy_device_map[eui64_str].push(frame);
  }

  return status;
}

sl_status_t zigpc_gateway_retrieve_sleepy_frame(const zigbee_eui64_t eui64,
                                                gateway_frame_t *frame)
{
  sl_status_t status = SL_STATUS_OK;

  if ((eui64 != NULL) && (frame != NULL)) {
    char eui64_cstr[ZIGBEE_EUI64_HEX_STR_LENGTH];
    zigbee_eui64_to_str(eui64, eui64_cstr, ZIGBEE_EUI64_HEX_STR_LENGTH);
    std::string eui64_str(eui64_cstr);

    auto found_device = sleepy_device_map.find(eui64_str);

    if (found_device != sleepy_device_map.end()) {
      if (!found_device->second.empty()) {
        gateway_frame_t next_frame = found_device->second.front();
        memcpy(frame, &next_frame, sizeof(gateway_frame_t));
        found_device->second.pop();  //remove next_frame

        status = SL_STATUS_OK;
      } else {
        status = SL_STATUS_EMPTY;
      }
    } else {
      status = SL_STATUS_EMPTY;
    }

  } else {
    status = SL_STATUS_FAIL;
  }

  return status;
}

unsigned int zigpc_gateway_retrieve_num_stored(const zigbee_eui64_t eui64)
{
  unsigned int size = 0;

  if (eui64 != NULL) {
    char eui64_cstr[ZIGBEE_EUI64_HEX_STR_LENGTH];
    zigbee_eui64_to_str(eui64, eui64_cstr, ZIGBEE_EUI64_HEX_STR_LENGTH);
    std::string eui64_str(eui64_cstr);

    auto found_device = sleepy_device_map.find(eui64_str);

    if (found_device != sleepy_device_map.end()) {
      size = found_device->second.size();
    } else {
      size = 0;
    }

  } else {
    size = 0;
  }

  return size;
}
