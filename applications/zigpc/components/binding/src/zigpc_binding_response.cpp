#include <algorithm>
#include <vector>

#include "sl_log.h"

#include "zigpc_gateway_notify.h"
#include "zigpc_datastore.h"
#include "zigpc_binding_mqtt.h"
#include "zigpc_binding_response.h"

const char LOG_TAG[] = "zigpc_binding_response";

void zigpc_binding_bind_unbind_callback(void *data)
{
        
    sl_log_debug(LOG_TAG, "Received bind/unbind response" );

    zigpc_gateway_on_bind_unbind_reponse_t response_data =
        *((zigpc_gateway_on_bind_unbind_reponse_t*)data);

    zigpc_binding_handle_bind_unbind_reponse(
                response_data.binding,
                response_data.is_bind_response,
                response_data.zdo_status);
}

sl_status_t zigpc_binding_handle_bind_unbind_reponse(
                const zigbee_binding_t binding, //NOSONAR - do not pass by reference to use in c code
                bool is_bind_response,
                uint8_t zdo_status)
{
    sl_status_t status = SL_STATUS_OK;
    
    //zdo_success
    if(0 == zdo_status)
    {
        sl_log_debug(LOG_TAG, "Handle ZDO bind or unbind request" );
        
        if(is_bind_response)
        {
            sl_log_info(LOG_TAG, "Request binding" );
            status = zigpc_binding_add_binding(binding);
        }
        else
        {
            sl_log_info(LOG_TAG, "Request unbinding" );
            status = zigpc_binding_remove_binding(binding);
            
            zigpc_binding_mqtt_publish_table_full(
                        binding.source_address, 
                        binding.source_endpoint,
                        true, 
                        false);
        }
    }
    //zdo_table full
    else if((ZDO_TABLE_FULL== zdo_status) || (ZDO_DEVICE_BIND_TABLE_FULL== zdo_status))
    {
            
        sl_log_info(
                LOG_TAG, 
                "Binding table is full for address EUI64 %llx - binding not saved", 
                binding.source_address);

        zigpc_binding_mqtt_publish_table_full(
            binding.source_address, 
            binding.source_endpoint,
            true, 
            true);
    }
    //error
    else
    {
            
        sl_log_warning(LOG_TAG, "Error in ZDO bind response with ZDO status: 0x%02X", zdo_status );
        status = SL_STATUS_FAIL;
    }

    return status;
}


bool operator==(const zigbee_binding_t &A,const zigbee_binding_t &B)
{
    bool result =   (A.source_address == B.source_address) &&
                    (A.source_endpoint == B.source_endpoint) &&
                    (A.source_cluster == B.source_cluster) &&
                    (A.dest_address == B.dest_address) &&
                    (A.dest_endpoint == B.dest_endpoint);
    return result;
            
}

bool operator!=(const zigbee_binding_t &A,const zigbee_binding_t &B)
{
    return !(A == B);
}


sl_status_t zigpc_binding_remove_binding(const zigbee_binding_t binding) //NOSONAR - do not pass by reference to call using c
{
    zigbee_eui64_t eui64 = {0,0,0,0,0,0,0,0};

    sl_status_t status = 
        zigbee_uint_to_eui64(binding.source_address, eui64);
    
    if(SL_STATUS_OK != status)
    {
        return status;
    }

    size_t binding_list_size =
                zigpc_datastore_get_binding_count(
                    eui64,
                    binding.source_endpoint);
    
    if(binding_list_size == 0)
    {
        //if first one, set to 1 to allow a list to exist
        binding_list_size = 1;
    }

    zigbee_binding_t binding_list[binding_list_size]; //NOSONAR - need to use a c array to interact with the datastore


    status = zigpc_datastore_read_binding_list( 
                eui64,
                binding.source_endpoint,
                binding_list,
                binding_list_size);

    if(SL_STATUS_OK != status)
    {
        return status;
    }

    std::vector<zigbee_binding_t> updated_list;
    
    for(size_t i = 0; i<binding_list_size; i++)
    {
        //copy every binding EXCEPT the one to remove
       if(binding_list[i] != binding )
       {
           updated_list.push_back(binding_list[i]);
       }
    }

    size_t updated_size = updated_list.size();
    zigbee_binding_t updated_array[updated_size]; //NOSONAR - need to use a c array to interact with datastore
    std::copy(
            updated_list.begin(),
            updated_list.end(),
            updated_array);

    status = zigpc_datastore_write_binding_list( 
                eui64,
                binding.source_endpoint,
                updated_array,
                updated_size);
    
    if(SL_STATUS_OK != status)
    {
        return status;
    }

    status = zigpc_publish_binding_table(
                binding.source_address, 
                binding.source_endpoint);

    return status;
}

sl_status_t zigpc_binding_add_binding(const zigbee_binding_t binding) //NOSONAR - do not pass by reference as this is defined in C
{
    zigbee_eui64_t eui64;

    sl_status_t status = 
        zigbee_uint_to_eui64(binding.source_address, eui64);
    
    if(SL_STATUS_OK != status)
    {
        return status;
    }

    size_t binding_list_size =
                zigpc_datastore_get_binding_count(
                    eui64,
                    binding.source_endpoint);
   
    if(binding_list_size == 0)
    {
        //if first one, set to 1 to allow a list to exist
        binding_list_size = 1;
    }

    zigbee_binding_t binding_list[binding_list_size]; //NOSONAR - need to use c array to interact with datastore

    status = zigpc_datastore_read_binding_list( 
                eui64,
                binding.source_endpoint,
                binding_list,
                binding_list_size);

    if(SL_STATUS_OK != status)
    {
        return status;
    }

    std::vector<zigbee_binding_t> updated_list;
    
    updated_list.push_back(binding);
    for(size_t i = 0; i<binding_list_size; i++)
    {
        updated_list.push_back(binding_list[i]);
    }

    size_t updated_size = updated_list.size();
    zigbee_binding_t updated_array[updated_size];
    std::copy(
            updated_list.begin(),
            updated_list.end(),
            updated_array);

    status = zigpc_datastore_write_binding_list( 
                eui64,
                binding.source_endpoint,
                updated_array,
                updated_size);
    
    if(SL_STATUS_OK != status)
    {
        return status;
    }

    status = zigpc_publish_binding_table(
                binding.source_address, 
                binding.source_endpoint);

    return status;
}

