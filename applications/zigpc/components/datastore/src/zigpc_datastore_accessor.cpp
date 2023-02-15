/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA);; available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include <iomanip>
#include <vector>

#include "zigpc_datastore_accessor.hpp"
#include "zigpc_datastore_id.h"

/**
 * @brief Helper to format data in hexadecimal notation.
 *
 * @tparam T
 * @param out
 * @param width
 * @param data
 * @return std::ostream&
 */
template<typename T>
inline void ss_append_hex(std::ostream &out, int width, T data)
{
  out << std::hex << std::uppercase << std::setw(width) << std::setfill('0')
      << data;
}

NetworkAccessor::NetworkAccessor() :
  StoreAccessor<uint8_t, zigpc_network_data_t>(ZIGPC_DS_TYPE_NETWORK, "Network")
{}

std::string NetworkAccessor::to_str(attribute_store_node_t parent,
                                    uint8_t index)
{
  std::ostringstream ss;
  zigpc_network_data_t net;

  sl_status_t status = this->read(parent, index, &net);

  if (status == SL_STATUS_OK) {
    ss << "PCEUI64:";
    ss_append_hex<zigbee_eui64_uint_t>(ss,
                                       16,
                                       zigbee_eui64_to_uint(net.gateway_eui64));

    ss << ","
       << "PanId:0x";
    ss_append_hex<zigbee_panid_t>(ss, 4, net.panid);

    // ss << "ExtPanId:" << net.ext_panid << ",";

    ss << ","
       << "RadioChannel:" << std::dec << static_cast<int>(net.radio_channel)
       << "";
  }

  return ss.str();
}

DeviceAccessor::DeviceAccessor() :
  StoreAccessor<zigbee_eui64_uint_t, zigpc_device_data_t>(ZIGPC_DS_TYPE_DEVICE,
                                                          "Device")
{}

std::string DeviceAccessor::to_str(attribute_store_node_t parent,
                                   zigbee_eui64_uint_t eui64)
{
  std::ostringstream ss;
  zigpc_device_data_t dev;

  sl_status_t status = this->read(parent, eui64, &dev);

  if (status == SL_STATUS_OK) {
    ss << "EUI64:";
    ss_append_hex<zigbee_eui64_uint_t>(ss, 16, eui64);
    ss << ",NetStatus:" << std::hex << dev.network_status;
    ss << ",MaxCmdDelay:" << std::dec << dev.max_cmd_delay;
    ss << ",EPDiscovered:" << std::dec
       << static_cast<int>(dev.endpoint_discovered_count);
    ss << ",EPTotal:" << std::dec << static_cast<int>(dev.endpoint_total_count);
  }

  return ss.str();
}

EndpointAccessor::EndpointAccessor() :
  StoreAccessor<zigbee_endpoint_id_t, zigpc_endpoint_data_t>(
    ZIGPC_DS_TYPE_ENDPOINT, "Endpoint")
{}

std::string EndpointAccessor::to_str(attribute_store_node_t parent,
                                     zigbee_endpoint_id_t endpoint_id)
{
  std::ostringstream ss;
  zigpc_endpoint_data_t ep;

  sl_status_t status = this->read(parent, endpoint_id, &ep);
  if (status == SL_STATUS_OK) {
    ss << "EndpointId:" << std::dec << static_cast<int>(endpoint_id);
  }

  return ss.str();
}

GroupAccessor::GroupAccessor(bool is_reported) :
  StoreAccessor<zigbee_group_id_t, zigpc_group_data_t>(
    get_entity_type(is_reported), get_label_type(is_reported)),
  is_reported(is_reported)
{}

std::string GroupAccessor::to_str(attribute_store_node_t parent,
                                  zigbee_group_id_t group_id)
{
  std::ostringstream ss;
  zigpc_group_data_t group_data;

  sl_status_t status = this->read(parent, group_id, &group_data);

  if (SL_STATUS_OK == status) {
    ss << get_label_type(this->is_reported) << ": GroupId:" << std::dec
       << static_cast<int>(group_id);
  }

  return ss.str();
}

ClusterAccessor::ClusterAccessor(zcl_cluster_side_t cluster_side) :
  StoreAccessor<zcl_cluster_id_t, zigpc_cluster_data_t>(
    get_entity_type(cluster_side), get_label_type(cluster_side)),
  cluster_side(cluster_side)
{}

std::string ClusterAccessor::to_str(attribute_store_node_t parent,
                                    zcl_cluster_id_t cluster_id)
{
  std::ostringstream ss;
  zigpc_cluster_data_t cl;

  sl_status_t status = this->read(parent, cluster_id, &cl);
  if (status == SL_STATUS_OK) {
    ss << get_label_type(this->cluster_side) << ": ClusterId:0x";
    ss_append_hex<zigbee_eui64_uint_t>(ss, 4, cluster_id);
  }

  return ss.str();
}

ClusterAttributeListAccessor::ClusterAttributeListAccessor() :
  StoreListAccessor<zcl_attribute_id_t>(ZIGPC_DS_TYPE_CLUSTER_ATTRIBUTE,
                                        get_label_type())
{}

std::string ClusterAttributeListAccessor::to_str(attribute_store_node_t parent,
                                                 size_t list_count)
{
  std::ostringstream ss;

  ss << get_label_type() << "List:[";
  if (list_count > 0) {
    std::vector<zcl_attribute_id_t> attributes(list_count);
    sl_status_t status
      = this->read_list(parent, attributes.data(), attributes.size());
    if (status == SL_STATUS_OK) {
      const char *sep = "";
      for (const zcl_attribute_id_t &attr: attributes) {
        ss << sep << "0x";
        ss_append_hex<zcl_attribute_id_t>(ss, 4, attr);
        sep = ",";
      }
    }
  }
  ss << "]";

  return ss.str();
}

BindingListAccessor::BindingListAccessor() :
  StoreListAccessor<std::string>(ZIGPC_DS_TYPE_BINDING,
                                        get_label_type())
{}

std::string BindingListAccessor::to_str(attribute_store_node_t parent,
                                                 size_t list_count)
{
  std::ostringstream ss;
  ss << get_label_type() << "List:[";
  if (list_count > 0) {
      std::vector<std::string> bindings(list_count);
    sl_status_t status
      = this->read_list(parent, bindings.data(), bindings.size());
    
    if(SL_STATUS_OK == status)
    {
        for(const std::string &bind: bindings)
        {
            ss << bind << ",";
        }
    }
  }

  ss << "]";
  return ss.str();
}

ClusterCommandListAccessor::ClusterCommandListAccessor(
  zcl_command_type_t command_type) :
  StoreListAccessor<zcl_command_id_t>(get_entity_type(command_type),
                                      get_label_type(command_type)),
  command_type(command_type)
{}

std::string ClusterCommandListAccessor::to_str(attribute_store_node_t parent,
                                               size_t list_count)
{
  std::ostringstream ss;

  ss << get_label_type(this->command_type) << "List:[";
  if (list_count > 0) {
    std::vector<zcl_command_id_t> commands(list_count);

    sl_status_t status
      = this->read_list(parent, commands.data(), commands.size());
    if (status == SL_STATUS_OK) {
      const char *sep = "";
      for (const zcl_command_id_t &cmd: commands) {
        ss << sep << "0x";
        ss_append_hex<int>(ss, 2, static_cast<int>(cmd));
        sep = ",";
      }
    }
  }
  ss << "]";

  return ss.str();
}
