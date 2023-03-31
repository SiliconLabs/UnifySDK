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
#include <iomanip>
#include <sstream>
#include <nlohmann/json.hpp>

// Unify libraries
#include <sl_log.h>
#include <sl_status.h>
#include <uic_mqtt.h>
#include <dotdot_cluster_id_definitions.h>

// ZigPC includes
#include <zigpc_common_unid.h>
#include <zigpc_datastore.hpp>

// Component includes
#include "zigpc_ucl.hpp"
#include "zigpc_ucl_int.hpp"

#define SS_FORMAT_HEX(width) \
  std::hex << std::uppercase << std::setw(width) << std::setfill('0')

namespace topic_part
{
//TODO Replace with strings from zigpc_common_unid.h
constexpr std::string_view UCL        = "ucl/";
constexpr std::string_view BY_UNID    = "by-unid/";
constexpr std::string_view BY_EP      = "/ep";
constexpr std::string_view NODE_STATE = "/State";
constexpr std::string_view PC_NWMGMT  = "/ProtocolController/NetworkManagement";
}  // namespace topic_part

static const std::string UNID_PREFIX = ZIGPC_UNID_PREFIX;

std::string zigpc_ucl::mqtt::build_unid(zigbee_eui64_uint_t eui64)
{
  std::stringstream eui64_ss;

  eui64_ss << UNID_PREFIX << SS_FORMAT_HEX(16) << eui64;

  return eui64_ss.str();
}

sl_status_t zigpc_ucl::mqtt::parse_unid(std::string unid,
                                        zigbee_eui64_uint_t &eui64)
{
  sl_status_t status = SL_STATUS_OK;
  // Check if prefix is first
  if (unid.size() < UNID_PREFIX.size()) {
    status = SL_STATUS_INVALID_SIGNATURE;
  } else if (UNID_PREFIX == unid.substr(0, UNID_PREFIX.size() + 1)) {
    status = SL_STATUS_INVALID_SIGNATURE;
  } else {
    unid = unid.substr(UNID_PREFIX.size());
    if (unid.size() < (ZIGBEE_EUI64_SIZE * 2)) {
      status = SL_STATUS_INVALID_SIGNATURE;
    }
  }

  // Check if buffer only has hexadecimal characters
  if (status == SL_STATUS_OK) {
    for (char c: unid) {
      if (!std::isxdigit(c)) {
        status = SL_STATUS_INVALID_SIGNATURE;
        break;
      }
    }
  }

  if (status == SL_STATUS_OK) {
    std::stringstream unid_ss(unid);

    unid_ss >> std::hex >> eui64;
    if (unid_ss.fail()) {
      status = SL_STATUS_INVALID_SIGNATURE;
      eui64  = 0U;
    }
  }

  return status;
}

sl_status_t zigpc_ucl::mqtt::parse_endpoint(
                                std::string endpoint_str, 
                                zigbee_endpoint_id_t &endpoint)
{
    sl_status_t status = SL_STATUS_OK;
    size_t raw_endpoint_value = 0;

    std::string ep_prefix("ep");

    //check if first string matches the expected prefix
    int comp_result = endpoint_str.compare(
                        0,                  //start index in endpoint_str
                        ep_prefix.size(),   //size of substring starting at index
                        ep_prefix);         //string to compare to
        
    sl_log_info(zigpc_ucl::LOG_TAG,"topic:%s", endpoint_str.c_str());
    sl_log_info(zigpc_ucl::LOG_TAG,"comp:%d", comp_result);

    if(0 == comp_result)
    {
        //strip "ep"
        std::string raw_endpoint_id = 
            endpoint_str.substr(
                            ep_prefix.size(),   //start of numbers
                            endpoint_str.size()-ep_prefix.size()); //length of remaining string
    
        sl_log_info(zigpc_ucl::LOG_TAG,"raw ep:%s", raw_endpoint_id.c_str());
        
        try
        {
            //try to convert resulting string to a number
            //raw_endpoint_id should be of format XXX
            raw_endpoint_value = std::stoul(raw_endpoint_id);
            status = SL_STATUS_OK;
        }
        catch(const std::exception &e)
        {
            status = SL_STATUS_FAIL;
        }
    }
    else
    {
        sl_log_warning(zigpc_ucl::LOG_TAG,"Failed to find endpoint in topic:%s, with result:%d", endpoint_str.c_str(), comp_result);
        status = SL_STATUS_FAIL;
    }

    //check for overflows
    if(SL_STATUS_OK == status)
    {
        //by ZCL spec, max endpoint is 255
        if(raw_endpoint_value < ZIGBEE_MAX_ENDPOINT_COUNT)
        {
            endpoint = raw_endpoint_value;
        }
        else
        {
            status = SL_STATUS_FAIL;
        }
    }

    return status;
}

sl_status_t 
    zigpc_ucl::mqtt::parse_topic_eui64(
                    const std::string topic, 
                    zigbee_eui64_uint_t &eui64)
{
    sl_status_t status = SL_STATUS_OK;

    const std::string total_ucl_prefix =
                        std::string(UCL_TOP_LEVEL) + 
                        std::string(UCL_BY_UNID);

    const unsigned int unid_size = 
                        ZIGPC_UNID_PREFIX_LENGTH + 
                        ZIGPC_EUI64_STRING_LENGTH ; 

    //check if the input is large enough to attempt to parse
    if(topic.size() > (total_ucl_prefix.size()+unid_size))
    {
        std::string unid = topic.substr(
                               total_ucl_prefix.size(),    //start index
                               unid_size);                 //length

        status = zigpc_ucl::mqtt::parse_unid(unid, eui64); 
    }
    else
    {
        sl_log_warning(zigpc_ucl::LOG_TAG, "Error with MQTT topic size");
        status = SL_STATUS_FAIL;
    }
    
    return status;
}

sl_status_t 
    zigpc_ucl::mqtt::parse_topic_endpoint(
                        const std::string topic, 
                        zigbee_endpoint_id_t &endpoint)
{
    sl_status_t status = SL_STATUS_OK;

    size_t endpoint_found_index = topic.find("/ep");

    if(endpoint_found_index != std::string::npos) 
    {
        //retrieve epXXX as a string
        size_t start_index = endpoint_found_index + 1; //ignore first '/'
        size_t rem_length = topic.size() - start_index;
        std::string topic_half = topic.substr(start_index, rem_length);
        std::string ep_string;

        std::stringstream ss(topic_half);
        //get ep_string in form "epXXX"
        getline(ss, ep_string, '/'); //get contents until next '/'

        //retrieve the endpoint ID
        status = zigpc_ucl::mqtt::parse_endpoint(ep_string, endpoint);
    }
    else
    {
        status = SL_STATUS_FAIL;
    }

    return status;
}
sl_status_t 
    zigpc_ucl::mqtt::parse_topic_cluster(
                        const std::string topic, 
                        std::string &cluster_name)
{
    sl_status_t status = SL_STATUS_OK;
    std::string stripped("");
    
    const std::string total_ucl_prefix =
                        std::string(UCL_TOP_LEVEL) + 
                        std::string(UCL_BY_UNID);

    const unsigned int unid_size = 
                        ZIGPC_UNID_PREFIX_LENGTH + 
                        ZIGPC_EUI64_STRING_LENGTH ; 
    
    size_t start_index = total_ucl_prefix.size() + unid_size + 1; //add one for '/'

    if(topic.size() > start_index)
    {
        stripped = topic.substr(
                             start_index,
                             topic.size()-start_index);
    }
    else
    {
        status = SL_STATUS_FAIL;
    }

    if(SL_STATUS_OK == status)
    {
        //now remove the endpoint
        std::string ep_string;
        std::stringstream s(stripped);
        getline(s, ep_string, '/'); //get contents until next '/'
        getline(s, cluster_name, '/'); //get contents until next '/'

        //if the size is 0, then it was not available
        if(cluster_name.size() > 0)
        {
            status = SL_STATUS_OK;
        }
        else
        {
            status = SL_STATUS_FAIL;
        }
    }

    return status;
}

sl_status_t zigpc_ucl::mqtt::parse_binding(
                const std::string topic, 
                const std::string payload, 
                zigbee_eui64_uint_t &source_eui64,
                zigbee_endpoint_id_t &source_ep,
                std::string &cluster_name,
                zigbee_eui64_uint_t &dest_eui64,
                zigbee_endpoint_id_t &dest_ep)
{
    sl_status_t status = SL_STATUS_OK;
        
    sl_log_info(zigpc_ucl::LOG_TAG,"Attempting to parse topic:%s", topic.c_str());

    nlohmann::json json_object;


    status = 
        zigpc_ucl::mqtt::parse_topic_eui64(topic, source_eui64);
    
    if(SL_STATUS_OK != status)
    {
        sl_log_warning(zigpc_ucl::LOG_TAG,
                     "Failed to parse topic eui64 with status:%x and topic:%s", status, topic);
        return status;
    }
   
        status = 
            zigpc_ucl::mqtt::parse_topic_endpoint(topic, source_ep);

    if(SL_STATUS_OK != status)
    {
        sl_log_warning(zigpc_ucl::LOG_TAG,
                     "Failed to parse topic endpoint with status:%x and topic:%s", status, topic);
        return status;
    }
    
    status = zigpc_ucl::mqtt::parse_payload(payload.c_str(), json_object);
    if(SL_STATUS_OK != status)
    {
        sl_log_warning(zigpc_ucl::LOG_TAG,
                     "Failed to parse payload with status:%x and payload:%s", status, topic);
        return status;
    }


    //JSON operations can throw exceptions
    //fails if the argument is not found or if unable to parse to
    //requested type
    try
    {
        cluster_name = json_object["ClusterName"].get<std::string>();
        dest_ep  = json_object["DestinationEp"].get<uint16_t>();
        std::string dest_unid 
                = json_object["DestinationUnid"].get<std::string>();
        status = zigpc_ucl::mqtt::parse_unid(dest_unid,dest_eui64);
         
    }
    catch(nlohmann::json::parse_error& ex)
    {
        sl_log_warning(zigpc_ucl::LOG_TAG,
                     "Failed to parse JSON with status:%x and topic:%s", status, topic.c_str());
        status = SL_STATUS_FAIL;
    }

    return status ;
}

sl_status_t
  zigpc_ucl::mqtt::build_topic(zigpc_ucl::mqtt::topic_type_t topic_type,
                               zigpc_ucl::mqtt::topic_data_t topic_data,
                               std::string &topic)
{
  sl_status_t status = SL_STATUS_OK;

  std::stringstream topic_ss;
  topic_ss << topic_part::UCL;

  std::string unid = zigpc_ucl::mqtt::build_unid(topic_data.eui64);
  switch (topic_type) {
    case topic_type_t::BY_UNID_PC_NWMGMT:
      topic_ss << topic_part::BY_UNID << unid << topic_part::PC_NWMGMT;
      break;
    case topic_type_t::BY_UNID_PC_NWMGMT_WRITE:
      topic_ss << topic_part::BY_UNID << unid << topic_part::PC_NWMGMT
               << "/Write";
      break;
    case topic_type_t::BY_UNID_NODE:
      topic_ss << topic_part::BY_UNID << unid;
      break;
    case topic_type_t::BY_UNID_NODE_EP:
      topic_ss << topic_part::BY_UNID << unid << topic_part::BY_EP << std::dec
               << static_cast<int>(topic_data.endpoint_id);
      break;
    case topic_type_t::BY_UNID_NODE_STATE:
      topic_ss << topic_part::BY_UNID << unid << topic_part::NODE_STATE;
      break;
    default:
      status = SL_STATUS_INVALID_TYPE;
      sl_log_warning(zigpc_ucl::LOG_TAG,
                     "Unknown topic to subscribe (%u): 0x%X",
                     topic_type,
                     status);
      break;
  }

  if (status == SL_STATUS_OK) {
    topic = topic_ss.str();
  }

  return status;
}

sl_status_t 
    zigpc_ucl::mqtt::publish_supported_generated(
        zigbee_eui64_uint_t eui64,
        zigbee_endpoint_id_t endpoint_id)
{
    sl_status_t status = SL_STATUS_OK;
  
    std::string unid = zigpc_ucl::mqtt::build_unid(eui64);
  
    std::stringstream topic_ss;
    topic_ss << UCL_TOP_LEVEL;
    topic_ss << UCL_BY_UNID;
    topic_ss << unid;
    topic_ss << UCL_EP_PREFIX; 
    topic_ss << std::to_string(endpoint_id) << "/"; 

    std::string topic_base = topic_ss.str();

    zigbee_eui64_t eui64_array;

    status = zigbee_uint_to_eui64(eui64, eui64_array);

    std::vector<zcl_cluster_id_t> client_cluster_list =
       zigpc_datastore::cluster::get_id_list(
               eui64_array, 
               endpoint_id, 
               ZCL_CLUSTER_CLIENT_SIDE);

    for(zcl_cluster_id_t cluster_id: client_cluster_list)
    {

        const char* cluster_name = 
            uic_dotdot_get_cluster_name(cluster_id);

        if(cluster_name != nullptr)
        {
            std::string empty_payload = "{}";
            std::string cluster_name_str = std::string(cluster_name);

            std::string full_topic = topic_base + cluster_name_str;
    
            uic_mqtt_publish(
                    full_topic.c_str(), 
                    empty_payload.c_str(), 
                    empty_payload.size(), 
                    true);
        }

    }

    return status;
}

sl_status_t zigpc_ucl::mqtt::subscribe(zigpc_ucl::mqtt::topic_type_t topic_type,
                                       zigpc_ucl::mqtt::topic_data_t topic_data,
                                       mqtt_message_callback_t cb)
{
  sl_status_t status = SL_STATUS_OK;

  if (cb == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    std::string topic;

    status = zigpc_ucl::mqtt::build_topic(topic_type, topic_data, topic);
    if (status == SL_STATUS_OK) {
      uic_mqtt_subscribe(topic.c_str(), cb);
    }
  }

  return status;
}

sl_status_t zigpc_ucl::mqtt::publish(zigpc_ucl::mqtt::topic_type_t topic_type,
                                     zigpc_ucl::mqtt::topic_data_t topic_data,
                                     const char *payload,
                                     size_t payload_size,
                                     bool retain)
{

  std::string topic;

  sl_status_t status
    = zigpc_ucl::mqtt::build_topic(topic_type, topic_data, topic);
  if (status == SL_STATUS_OK) {
    uic_mqtt_publish(topic.c_str(), payload, payload_size, retain);
  }
  else
  {
    sl_log_warning(zigpc_ucl::LOG_TAG,"Failed to build desired MQTT topic");
  }

  return status;
}

sl_status_t zigpc_ucl::mqtt::unretain(zigpc_ucl::mqtt::topic_type_t topic_type,
                                      zigpc_ucl::mqtt::topic_data_t topic_data)
{
  std::string topic;

  sl_status_t status
    = zigpc_ucl::mqtt::build_topic(topic_type, topic_data, topic);
  if (status == SL_STATUS_OK) {
    uic_mqtt_unretain(topic.c_str());
  }

  return status;
}

sl_status_t zigpc_ucl::mqtt::parse_payload(const char *payload, nlohmann::json &jsn)
{
  sl_status_t status = SL_STATUS_OK;

  if (payload == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  } else if (std::char_traits<char>::length(payload) == 0) {
    status = SL_STATUS_EMPTY;
  }

  if (status == SL_STATUS_OK) {
    try {
      jsn = nlohmann::json::parse(payload);
    } catch (const nlohmann::json::parse_error& err) {
      sl_log_warning(zigpc_ucl::LOG_TAG,
                     "Failed to parse payload: %s",
                     err.what());
      status = SL_STATUS_OBJECT_READ;
    }
  }

  return status;
}
