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
#include <string>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

// UIC libraries
#include <sl_log.h>
#include <sl_status.h>

// ZigPC includes
#include <zigpc_datastore.h>

// Component includes
#include "zigpc_ucl_fixt.h"
#include "zigpc_ucl_int.hpp"

/**
 * @brief Event handler for Net Mgmt init
 *
 * @param event_data event data containing network information
 */
void zigpc_ucl_on_nwmgmt_init(void *event_data)
{
  if (event_data != nullptr) {
    zigpc_net_mgmt_on_network_init_t &net_init
      = *reinterpret_cast<struct zigpc_net_mgmt_on_network_init *>(event_data);
    zigpc_device_data_t data;

    zigbee_eui64_uint_t pc_eui64 = zigbee_eui64_to_uint(net_init.pc_eui64);

    sl_status_t status = zigpc_datastore_read_device(net_init.pc_eui64, &data);

    if (status == SL_STATUS_OK) {
      status = zigpc_ucl::pc_nwmgmt::on_net_init(pc_eui64);
    }

    if (status == SL_STATUS_OK) {
      status = zigpc_ucl::node_state::publish_state(pc_eui64,
                                                    data.network_status,
                                                    data.max_cmd_delay);
    }
    if (status != SL_STATUS_OK) {
      sl_log_warning(zigpc_ucl::LOG_TAG,
                     "Network init event handler status: 0x%X",
                     status);
    }
  }
}

/**
 * @brief Event handler for Net Mgmt state updates
 *
 * @param event_data event data containing new state information
 */
void zigpc_ucl_on_nwmgmt_state_update(void *event_data)
{
  if (event_data != nullptr) {
    zigpc_net_mgmt_on_network_state_update_t &state_update
      = *reinterpret_cast<zigpc_net_mgmt_on_network_state_update_t *>(
        event_data);

    sl_status_t status
      = zigpc_ucl::pc_nwmgmt::on_net_state_update(state_update);
    if (status != SL_STATUS_OK) {
      sl_log_warning(zigpc_ucl::LOG_TAG,
                     "Network state update event handler status: 0x%X",
                     status);
    }
  }
}

sl_status_t zigpc_ucl_fixt_setup(void)
{
  sl_status_t status;

  status = zigpc_net_mgmt_register_observer(ZIGPC_NET_MGMT_NOTIFY_NETWORK_INIT,
                                            zigpc_ucl_on_nwmgmt_init);

  if (status == SL_STATUS_OK) {
    status
      = zigpc_net_mgmt_register_observer(ZIGPC_NET_MGMT_NOTIFY_STATE_UPDATE,
                                         zigpc_ucl_on_nwmgmt_state_update);
  }

  if (status != SL_STATUS_OK) {
    sl_log_error(zigpc_ucl::LOG_TAG,
                 "Network management observer registrations failed: 0x%X",
                 status);
  }

  return status;
}

int zigpc_ucl_fixt_shutdown(void)
{
  zigpc_net_mgmt_unregister_observer(ZIGPC_NET_MGMT_NOTIFY_NETWORK_INIT,
                                     zigpc_ucl_on_nwmgmt_init);
  zigpc_net_mgmt_unregister_observer(ZIGPC_NET_MGMT_NOTIFY_STATE_UPDATE,
                                     zigpc_ucl_on_nwmgmt_state_update);
  return 0;
}
