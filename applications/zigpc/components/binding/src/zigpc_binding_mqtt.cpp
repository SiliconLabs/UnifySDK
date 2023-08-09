
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>

#include "sl_log.h"

#include "dotdot_cluster_id_definitions.h"

#include "zigpc_ucl.hpp"
#include "zigpc_common_unid.h"
#include "zigpc_datastore.hpp"
#include "zigpc_binding_mqtt_helpers.h"

#include "zigpc_binding_mqtt.h"
#include "zigpc_binding_request.h"

static const char LOG_TAG[] = "zigpc_binding_mqtt";

std::string 
    zigpc_binding_serialize_attribute_topic(
            zigbee_eui64_uint_t eui64,
            zigbee_endpoint_id_t endpoint)
{
    std::string unid = zigpc_ucl::mqtt::build_unid(eui64); 
    std::string endpoint_string = std::string(UCL_EP_PREFIX) + 
        std::to_string(endpoint);
    std::string topic_string =
        std::string(UCL_TOP_LEVEL) + std::string(UCL_BY_UNID) + unid 
        + endpoint_string + BINDING_CLUSTER_NAME
        + std::string(UCL_ATTRIBUTE_TOPIC) ;

    return topic_string;
}

sl_status_t zigpc_binding_init_mqtt(
        zigbee_eui64_uint_t eui64,
        zigbee_endpoint_id_t endpoint)
{
    sl_status_t status = SL_STATUS_OK;
    
    status = zigpc_binding_mqtt_pub_supported(eui64, endpoint);
    if(SL_STATUS_OK != status)
    {
        sl_log_warning(LOG_TAG, "Error initializing supported binding commands");
        return status;
    }
    
    status = zigpc_binding_mqtt_pub_generated(eui64, endpoint);

    if(SL_STATUS_OK != status)
    {
        sl_log_warning(LOG_TAG, "Error initializing generated binding commands");
        return status;
    }

    status = 
        zigpc_binding_mqtt_publish_table_full(
                eui64, 
                endpoint, 
                ATTR_DIRECTION_REPORTED, 
                false );

    if(SL_STATUS_OK != status)
    {
        sl_log_warning(LOG_TAG, "Error initializing BindingTableFull REPORTED attribute");
        return status;
    }

    status = 
        zigpc_binding_mqtt_publish_table_full(
            eui64, 
            endpoint, 
            ATTR_DIRECTION_DESIRED, 
            false);
    if(SL_STATUS_OK != status)
    {
        sl_log_warning(LOG_TAG, "Error initializing the BindingTableFull DESIRED attribute");
        return status;
    }

    status = zigpc_binding_mqtt_sub_bind(eui64, endpoint, zigpc_binding_parse_bind_cmd);
    if(SL_STATUS_OK != status)
    {
        sl_log_warning(LOG_TAG, "Error subscribing to the given binding callback");
        return status;
    }

    status = zigpc_binding_mqtt_sub_unbind(eui64, endpoint, zigpc_binding_parse_unbind_cmd);
    if(SL_STATUS_OK != status)
    {
        sl_log_warning(LOG_TAG, "Error subscribing to the given unbind callback");
        return status;
    }

    status = zigpc_binding_mqtt_publish_bindable_clusters(
                eui64,
                endpoint);
    return status;
}


sl_status_t zigpc_publish_binding_table(zigbee_eui64_uint_t eui64, zigbee_endpoint_id_t endpoint)
{
    //build topic
    std::string topic_string =  
            zigpc_binding_serialize_attribute_topic(eui64,endpoint) + 
            BINDING_TABLE_TOPIC + 
            UCL_ATTRIBUTE_REPORTED_TOPIC;

    //retrieve bindings
    std::list<zigbee_binding_t> binding_table;
    
    sl_status_t status = 
        zigpc_datastore::binding::read_binding_table(
            eui64,
            binding_table);
    
    //build payload
    std::stringstream payload("{\"value\":[");
    for(zigbee_binding_t binding : binding_table)
    {
        std::string cluster_name = 
            uic_dotdot_get_cluster_name(
                    binding.source_cluster);
        std::string dest_unid = 
            zigpc_ucl::mqtt::build_unid(
                    binding.dest_address);
        std::string ep_string = std::to_string(
                binding.dest_endpoint);

        payload << 
            "{\"ClusterName\" : " << "\"" << cluster_name << "\"" << "," <<
            "\"DestinationUnid\" : " << "\"" << dest_unid << "\"" << "," <<
            "\"DestinationEp\" : " << ep_string << "}" << ",";
    }

    payload << "]}";

    //publish
    
    uic_mqtt_publish(
          topic_string.c_str(),
          payload.str().c_str(),
          payload.str().size(),
          true);

    return status;
}

sl_status_t 
    zigpc_binding_mqtt_publish_table_full(
        zigbee_eui64_uint_t eui64,
        zigbee_endpoint_id_t endpoint,
        bool is_reported,
        bool table_full_value)
{
   
    std::string direction_str =
        is_reported ?
            "Reported" :
            "Desired";

    std::string table_full_value_str =
        table_full_value ?
            "true" :
            "false";
    
    std::string table_full_topic =  
            zigpc_binding_serialize_attribute_topic(eui64,endpoint) + 
            BINDING_TABLE_FULL_TOPIC + 
            direction_str;
    
    std::stringstream stream;
    stream << std::hex<< eui64;

    sl_log_info(
            LOG_TAG, 
            "Publish tablefull attribute for eui: %s, ep:%s, dir: %s",
            stream.str().c_str(),
            std::to_string(endpoint).c_str(),
            direction_str.c_str() );
    
    std::string table_full_payload = 
        "{" + std::string(UCL_VALUE) + ":" + table_full_value_str +"}";
    
    uic_mqtt_publish(
          table_full_topic.c_str(),
          table_full_payload.c_str(),
          table_full_payload.size(),
          true);
    
    return SL_STATUS_OK;
}

sl_status_t 
    zigpc_binding_mqtt_publish_bindable_clusters(
            zigbee_eui64_uint_t eui64,
            zigbee_endpoint_id_t endpoint)
{
    std::string topic_string =  
        zigpc_binding_serialize_attribute_topic(eui64,endpoint) + 
        BINDABLE_CLUSTERS_TOPIC  + UCL_ATTRIBUTE_REPORTED_TOPIC ;
    
    std::stringstream stream;
    stream << std::hex<< eui64;

    sl_log_info(
            LOG_TAG, 
            "Publish bindable cluster list for eui: %s, ep:%s",
            stream.str().c_str(),
            std::to_string(endpoint).c_str());

    zigbee_eui64_t eui64_array;

    sl_status_t status = 
        zigbee_uint_to_eui64(eui64, eui64_array);

    if(SL_STATUS_OK != status)
    {
        sl_log_warning(LOG_TAG, "Error using the source eui64 address");
        return status;
    }

    size_t cluster_count = 
            zigpc_datastore_get_cluster_count(
                eui64_array,
                endpoint,
                ZCL_CLUSTER_CLIENT_SIDE );
    
    std::vector<zcl_cluster_id_t> cluster_list;

    for(size_t i = 0; i < cluster_count; i++)
    {
        zcl_cluster_id_t cluster_id;
        status =
            zigpc_datastore_find_cluster_by_index(
                    eui64_array,
                    endpoint,
                    ZCL_CLUSTER_CLIENT_SIDE,
                    i,
                    &cluster_id);

        if(SL_STATUS_OK == status)
        {
            cluster_list.push_back(cluster_id);
        }
    }

    std::stringstream payload;
    payload << "{\"value\":[";

    bool first = true;
    for(auto cluster_id : cluster_list)
    {   
        if(!first) payload << ",";
        std::string cluster_name = 
            uic_dotdot_get_cluster_name(cluster_id);
        payload << "\"" << cluster_name << "\"" ;
        first = false;
    }
  
    
    
    payload << "]}";
    
    uic_mqtt_publish(
          topic_string.c_str(),
          payload.str().c_str(),
          payload.str().size(),
          true);

    //return OK if we make it here
    return SL_STATUS_OK;
}
