#include <string>
#include "sl_log.h"

#include "dotdot_cluster_id_definitions.h"

#include "zigpc_gateway.h"
#include "zigpc_ucl.hpp"
#include "zigpc_binding_request.h"
#include "zigpc_datastore.hpp"
#include <algorithm>


static const char LOG_TAG[] = "zigpc_binding_request";

void zigpc_binding_parse_bind_cmd(
    const char *topic,
    const char *message,
    const size_t message_length)
{
   
    zigbee_binding_t binding = NULL_BINDING;


    sl_status_t status =
        zigpc_binding_parse_cmd_args(
                topic,
                message,
                message_length,
                &binding);
    
    if(SL_STATUS_OK != status)
    {
        sl_log_warning(LOG_TAG, "Error parsing binding arguments with status: %x", status);
        return;
    }
    
    sl_log_debug(LOG_TAG, "topic is: %s", topic);
    sl_log_debug(LOG_TAG, "payload is: %s", message);

    zigbee_eui64_t source_eui_array = {0,0,0,0,0,0,0,0};
    zigbee_eui64_t dest_eui_array = {0,0,0,0,0,0,0,0};
    
    status = zigbee_uint_to_eui64(binding.source_address, source_eui_array);
    if(SL_STATUS_OK != status)
    {
        sl_log_warning(LOG_TAG, "Error parsing source address");
        return;
    }

    status = zigbee_uint_to_eui64(binding.dest_address, dest_eui_array);    
    if(SL_STATUS_OK != status)
    {
        sl_log_warning(LOG_TAG, "Error parsing destination address");
        return;
    }

    
    // check if the destination exist in the datastore :
    if(!zigpc_datastore_is_device_contained(binding.dest_address)){
        status = SL_STATUS_NOT_FOUND;
    }
    
    if(SL_STATUS_OK != status)
    {
        sl_log_warning(LOG_TAG, "Endpoint Destination not found");
        return;
    }

    //call zigpc_gateway_bind
    status = 
        zigpc_gateway_request_binding(
            source_eui_array,
            binding.source_endpoint,
            binding.source_cluster,
            dest_eui_array,
            binding.dest_endpoint);

    if(SL_STATUS_OK != status)
    {
        sl_log_warning(LOG_TAG, "Error sending the ZDO binding request");
    }
    return;
}

void zigpc_binding_parse_unbind_cmd(
    const char *topic,
    const char *message,
    const size_t message_length)
{
    zigbee_binding_t binding = NULL_BINDING;

    sl_status_t status =
        zigpc_binding_parse_cmd_args(
                topic,
                message,
                message_length,
                &binding);
    
    if(SL_STATUS_OK != status)
    {
        sl_log_warning(LOG_TAG, "Error parsing unbinding arguments");
        return ;
    }

    zigbee_eui64_t source_eui_array = {0,0,0,0,0,0,0,0};
    zigbee_eui64_t dest_eui_array = {0,0,0,0,0,0,0,0};
    
    status = zigbee_uint_to_eui64(binding.source_address, source_eui_array);
    if(SL_STATUS_OK != status)
    {
        sl_log_warning(LOG_TAG, "Error parsing unbinding source address ");
        return ;
    }

    status = zigbee_uint_to_eui64(binding.dest_address, dest_eui_array);
    if(SL_STATUS_OK != status)
    {
        sl_log_warning(LOG_TAG, "Error parsing unbinding dest address ");
        return ;
    }
    
    //call zigpc_gateway_bind
    status = 
        zigpc_gateway_request_unbind(
            source_eui_array,
            binding.source_endpoint,
            binding.source_cluster,
            dest_eui_array,
            binding.dest_endpoint);

    if(SL_STATUS_OK != status)
    {
        sl_log_warning(LOG_TAG, "Error sending ZDO for unbind request ");
    }
    return;
}

sl_status_t zigpc_binding_parse_cmd_args(
    const char *topic,
    const char *message,
    const size_t message_length,
    zigbee_binding_t *binding)
{
    sl_status_t status = SL_STATUS_OK;

    zigbee_eui64_uint_t source_eui64 = 0;
    zigbee_endpoint_id_t source_ep = 0;
    zigbee_eui64_uint_t dest_eui64 = 0;
    zigbee_endpoint_id_t dest_ep = 0;
   
    std::string cluster_name = "";
    zcl_cluster_id_t cluster_id = 0;

    if(binding == nullptr || topic == nullptr || message == nullptr )
    {
        sl_log_warning(LOG_TAG, "Error sending parsing binding arguments due to null input");
        status = SL_STATUS_NULL_POINTER;
        return status;
    }
    
    sl_log_info(LOG_TAG, "Attempting to parse topic: %s", topic);
    sl_log_info(LOG_TAG, "Attempting to parse payload: %s", message);

    std::string topic_str(topic);
    std::string payload_str(message);

    status = zigpc_ucl::mqtt::parse_binding(
                topic_str, 
                payload_str, 
                source_eui64,
                source_ep,
                cluster_name,
                dest_eui64,
                dest_ep);
    
    cluster_id = uic_dotdot_get_cluster_id(cluster_name.c_str());

    if(SL_STATUS_OK == status)
    {
        binding->source_address     = source_eui64;
        binding->source_endpoint    = source_ep;
        binding->source_cluster     = cluster_id;
        binding->dest_address       = dest_eui64;
        binding->dest_endpoint      = dest_ep;
    }
    else
    {
        sl_log_warning(LOG_TAG, "Error parsing binding information with status: %x", status);
    }

    return status;
}

