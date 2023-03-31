#include <string>
#include <list>
#include "uic_mqtt.h"

#include "sl_log.h"

#include "zigpc_common_unid.h"
#include "zigpc_ucl.hpp"

#include "zigpc_binding_mqtt_helpers.h"
 
static const std::string supported_commands_string ="[\"Bind\", \"Unbind\"]";

const char LOG_TAG[] = "bind_mqtt_helper";

sl_status_t zigpc_binding_mqtt_sub_bind(
        zigbee_eui64_uint_t eui64,
        zigbee_endpoint_id_t endpoint,
        mqtt_message_callback_t callback ) //NOSONAR - use definition from UIC components
{
    //Bind
    //build topic
    //ucl/by-unid/<UNID>/ep0/Commands/Bind

    std::string bind_cmd_topic = "";
    std::string unid = zigpc_ucl::mqtt::build_unid(eui64);
    bind_cmd_topic = 
        std::string(UCL_TOP_LEVEL) + std::string(UCL_BY_UNID) + unid 
        + std::string(UCL_EP_PREFIX) + std::to_string(endpoint) + BINDING_CLUSTER_NAME + 
        std::string(UCL_COMMAND_TOPIC)  + "/Bind"; 
    
    sl_log_info(
            LOG_TAG, 
            "Subscribe to BIND command for eui: %s, ep:%s",
            unid.c_str(),
            std::to_string(endpoint).c_str());

    uic_mqtt_subscribe(bind_cmd_topic.c_str(), callback);

    return SL_STATUS_OK;
}

sl_status_t zigpc_binding_mqtt_sub_unbind(
        zigbee_eui64_uint_t eui64,
        zigbee_endpoint_id_t endpoint,
        mqtt_message_callback_t callback)//NOSONAR - use definition from UIC components
{
    //Unbind
    //build topic
    //ucl/by-unid/<UNID>/ep0/Commands/Unbind

    std::string unbind_cmd_topic = "";
    std::string unid = zigpc_ucl::mqtt::build_unid(eui64);
    unbind_cmd_topic = 
        std::string(UCL_TOP_LEVEL) + std::string(UCL_BY_UNID) + unid 
        + std::string(UCL_EP_PREFIX) + std::to_string(endpoint) + BINDING_CLUSTER_NAME + 
        std::string(UCL_COMMAND_TOPIC)  + "/Unbind"; 
    
    sl_log_info(
            LOG_TAG, 
            "Subscribe to UNBIND command for eui: %s, ep:%s",
            unid.c_str(),
            std::to_string(endpoint).c_str());


    uic_mqtt_subscribe(unbind_cmd_topic.c_str(), callback);
    return SL_STATUS_OK;
}

sl_status_t zigpc_binding_mqtt_pub_supported(
        zigbee_eui64_uint_t eui64,
        zigbee_endpoint_id_t endpoint)
{

    if(eui64 == 0)
    {
        //0 is considered invalid input
        return SL_STATUS_FAIL;
    }
    //publish supported commands
    //ucl/by-unid/<UNID>/ep0/SupportedCommands
    //{"value":["Bind","Unbind"]}

    std::string supported_commands_topic = "";
    std::string unid = zigpc_ucl::mqtt::build_unid(eui64);
    supported_commands_topic  = 
        std::string(UCL_TOP_LEVEL) + std::string(UCL_BY_UNID) + unid 
        + std::string(UCL_EP_PREFIX) + std::to_string(endpoint) + BINDING_CLUSTER_NAME + 
        std::string(UCL_SUPPORTED_COMMAND_TOPIC );

    
    sl_log_info(
            LOG_TAG, 
            "Publish supported binding commands for eui: %s, ep:%s",
            unid.c_str(),
            std::to_string(endpoint).c_str());
    
    std::string supported_commands_payload = "";

    supported_commands_payload = "{" + std::string(UCL_VALUE) + ":" 
        + supported_commands_string + "}";

    uic_mqtt_publish(
            supported_commands_topic.c_str(),
            supported_commands_payload.c_str(),
            supported_commands_payload.size(),
            true);

    return SL_STATUS_OK;
}

sl_status_t zigpc_binding_mqtt_pub_generated(
        zigbee_eui64_uint_t eui64,
        zigbee_endpoint_id_t endpoint)
{
    //publish generated commands - EMPTY
    //ucl/by-unid/<UNID>/ep0/GeneratedCommands
    //{"value":[]}
    std::string generated_commands_topic = "";
    std::string unid = zigpc_ucl::mqtt::build_unid(eui64);
    generated_commands_topic  = 
        std::string(UCL_TOP_LEVEL) + std::string(UCL_BY_UNID) + unid 
        + std::string(UCL_EP_PREFIX) + std::to_string(endpoint) + BINDING_CLUSTER_NAME + 
        std::string(UCL_GENERATED_COMMAND_TOPIC );

    std::string generated_commands_payload = 
        "{" + std::string(UCL_VALUE) + ": []}";
    
    sl_log_info(
            LOG_TAG, 
            "Publish generated binding commands for eui: %s, ep:%s",
            unid.c_str(),
            std::to_string(endpoint).c_str());

    uic_mqtt_publish(
            generated_commands_topic.c_str(),
            generated_commands_payload.c_str(),
            generated_commands_payload.size(),
            true);

    return SL_STATUS_OK;
}
