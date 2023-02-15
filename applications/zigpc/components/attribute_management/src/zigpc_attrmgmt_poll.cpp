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

#include <cstring>
#include <algorithm>

#include <boost/circular_buffer.hpp>

#include "sl_log.h"
#include "sl_status.h"

#include "attribute_management_int.h"
#include "zigpc_attrmgmt_int.hpp"

static const char LOG_TAG[] = "zigpc_attrmgmt_poll";

//Internal structure to hold polling entries
typedef struct {
  zigbee_eui64_uint_t eui64;
  zigbee_endpoint_id_t endpoint_id;
  zcl_cluster_id_t cluster_id;
} attribute_poll_entry_t;

bool operator==(const attribute_poll_entry_t &A,
                const attribute_poll_entry_t &B)
{
  return (A.eui64 == B.eui64) && (A.endpoint_id == B.endpoint_id)
         && (A.cluster_id == B.cluster_id);
}

boost::circular_buffer<attribute_poll_entry_t>
  attribute_poll_list(ZIGPC_ATTRMGMT_DEFAULT_POLL_SIZE);

auto attribute_poll_iter = attribute_poll_list.begin();

size_t zigpc_attrmgmt_get_poll_list_current_size()
{
  return attribute_poll_list.size();
}

size_t zigpc_attrmgmt_get_poll_list_max_size()
{
  return attribute_poll_list.capacity();
}

bool zigpc_attrmgmt_poll_has_space()
{
  return !attribute_poll_list.full();
}

sl_status_t zigpc_attrmgmt_send_poll_attributes()
{
  sl_status_t status = SL_STATUS_OK;
  zigbee_eui64_t eui64_array;

  if (
          (attribute_poll_iter == attribute_poll_list.end()) &&
          (!attribute_poll_list.empty()))
  {
    sl_log_debug(LOG_TAG, "Reached end of poll list - reset index");

    attribute_poll_iter = attribute_poll_list.begin();
  }

  if (attribute_poll_iter != attribute_poll_list.end()) {
    status = zigbee_uint_to_eui64(attribute_poll_iter->eui64, eui64_array);

    if (SL_STATUS_OK == status) {
      sl_log_debug(LOG_TAG,
                   "Polling EUI64: %16" PRIX64 ", Endpoint: %u, Cluster: %04X",
                   attribute_poll_iter->eui64,
                   attribute_poll_iter->endpoint_id,
                   attribute_poll_iter->cluster_id);
      status = zigpc_attrmgmt_send_read_attributes_command(
        eui64_array,
        attribute_poll_iter->endpoint_id,
        attribute_poll_iter->cluster_id);
      std::advance(attribute_poll_iter, 1);
    } else {
      sl_log_warning(LOG_TAG, "Error parsing eui64");
    }
  } 
  

  return status;
}

sl_status_t
  zigpc_attrmgmt_get_next_poll_entry(zigbee_eui64_uint_t &eui64,
                                     zigbee_endpoint_id_t &endpoint_id,
                                     zcl_cluster_id_t &cluster_id)
{
  sl_status_t status = SL_STATUS_OK;
  unsigned int size  = zigpc_attrmgmt_get_poll_list_current_size();

  if (size == 0) {
    status = SL_STATUS_EMPTY;
  } else if (attribute_poll_iter == attribute_poll_list.end()) {
    status = SL_STATUS_INVALID_STATE;
  } else {
    status                       = SL_STATUS_OK;
    attribute_poll_entry_t entry = *attribute_poll_iter;

    eui64       = entry.eui64;
    endpoint_id = entry.endpoint_id;
    cluster_id  = entry.cluster_id;
  }

  return status;
}

sl_status_t zigpc_attrmgmt_add_poll_entry(const zigbee_eui64_t eui64,
                                          zigbee_endpoint_id_t endpoint_id,
                                          zcl_cluster_id_t cluster_id)
{
  sl_status_t status = SL_STATUS_OK;
  attribute_poll_entry_t entry;

  if ((eui64 != NULL) && (zigpc_attrmgmt_poll_has_space())) {
    entry.eui64       = zigbee_eui64_to_uint(eui64);
    entry.endpoint_id = endpoint_id;
    entry.cluster_id  = cluster_id;

    auto duplicate_check_iter = std::find(attribute_poll_list.begin(),
                                          attribute_poll_list.end(),
                                          entry);

    if (duplicate_check_iter == attribute_poll_list.end()) {
      attribute_poll_list.push_back(entry);

      //reset index if we modify the list
      attribute_poll_iter = attribute_poll_list.begin();
    } else {
      sl_log_debug(LOG_TAG, "Duplicate poll entry will not be added");
    }

    status = SL_STATUS_OK;
  } else {
    status = SL_STATUS_FAIL;
    sl_log_warning(LOG_TAG, "Error adding entry to attribute poll");
  }

  return status;
}

sl_status_t zigpc_attrmgmt_remove_poll_entry(const zigbee_eui64_t eui64,
                                             zigbee_endpoint_id_t endpoint_id,
                                             zcl_cluster_id_t cluster_id)
{
  sl_status_t status = SL_STATUS_OK;

  attribute_poll_entry_t entry;

  if (eui64 != nullptr) {
    entry.eui64       = zigbee_eui64_to_uint(eui64);
    entry.endpoint_id = endpoint_id;
    entry.cluster_id  = cluster_id;

    auto position = std::find(attribute_poll_list.begin(),
                              attribute_poll_list.end(),
                              entry);

    if (position != attribute_poll_list.end()) {
      attribute_poll_list.erase(position);
      //reset index if we modify the list
      attribute_poll_iter = attribute_poll_list.begin();
    }

  } else {
    status = SL_STATUS_FAIL;
  }

  return status;
}

void zigpc_attrmgmt_empty_poll_list()
{
  attribute_poll_list.clear();
  attribute_poll_iter = attribute_poll_list.begin();
}
