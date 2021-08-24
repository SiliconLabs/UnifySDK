/*******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "uic_mqtt.h"

#include "zigpc_node_map.hpp"
#include "zigpc_net_mgmt.h"
#include "zigpc_common_unid.h"
#include "zigpc_node_mgmt_int.h"

#include <vector>
#include <string>
#include <string.h>
void zigpc_node_mgmt_unretain_all_topics()
{
  std::vector<std::string> eui64_v = zigpc_node_map_get_all_eui64();

  // get Protocol controller UNID to unretain the Protocol controller messages
  zigbee_eui64_t pc_eui64;
  char ascii_eui64[ZIGBEE_EUI64_HEX_STR_LENGTH];
  zigpc_net_mgmt_get_protocol_controller_eui64(pc_eui64);
  zigbee_eui64_to_str(pc_eui64, ascii_eui64, ZIGBEE_EUI64_HEX_STR_LENGTH);
  eui64_v.push_back(std::string(ascii_eui64));

  for (auto const &eui64: eui64_v) {
    // get the UNID
    std::string unid = std::string(ZIGPC_UNID_PREFIX) + eui64;
    // unretain the node
    uic_mqtt_unretain(("ucl/by-unid/" + unid).c_str());
  }
}