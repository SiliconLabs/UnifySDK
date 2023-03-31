#include <algorithm>
#include <vector>
#include <list>

#include "sl_log.h"

#include "zigpc_common_util.hpp"
#include "zigpc_gateway_notify.h"
#include "zigpc_datastore.hpp"
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
            sl_log_info(LOG_TAG, "Request binding on address: %llx", binding.source_address);
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

sl_status_t zigpc_binding_remove_binding(const zigbee_binding_t binding) //NOSONAR - do not pass by reference to call using c
{
    
    sl_status_t status = 
        zigpc_datastore::binding::remove_binding( 
            binding.source_address,
            binding);

    status = zigpc_publish_binding_table(
                binding.source_address, 
                binding.source_endpoint);

    return status;
}

sl_status_t zigpc_binding_add_binding(const zigbee_binding_t binding) //NOSONAR - do not pass by reference as this is defined in C
{
    sl_status_t status = 
        zigpc_datastore::binding::add_binding( 
            binding.source_address,
            binding);

    status = zigpc_publish_binding_table(
                binding.source_address, 
                binding.source_endpoint);

    return status;
}

